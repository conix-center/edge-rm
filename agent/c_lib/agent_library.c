#include "agent_library.h"
#include "agent_port.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "pb_common.h"
#include "messages.pb.h"
#include <string.h>
#include <stdio.h>

const char* master_domain;
static uint32_t ping_rate;

#define RESPONSE_CODE_CONTENT 69
#define RESPONSE_CODE_VALID 67

#define TASK_ID_LEN 40
#define TASK_NAME_LEN 40
#define FRAMEWORK_ID_LEN 40
#define FRAMEWORK_NAME_LEN 40
#define ENVIRONMENT_KEY_LEN 40
#define ENVIRONMENT_VALUE_LEN 40
#define NUM_ENVIRONMENT_VARIABLES 10
typedef struct _agent_task {
    char task_name[TASK_NAME_LEN];
    char framework_name[FRAMEWORK_NAME_LEN];
    char task_id[TASK_ID_LEN];
    char framework_id[FRAMEWORK_ID_LEN];
    char* error_message;
    TaskInfo_TaskState state;
    uint8_t* wasm_binary;
    uint32_t wasm_binary_length;
    char environment_keys[NUM_ENVIRONMENT_VARIABLES][ENVIRONMENT_KEY_LEN];
    char * environment_key_pointers[NUM_ENVIRONMENT_VARIABLES];
    int32_t environment_values[NUM_ENVIRONMENT_VARIABLES];
    char environment_str_values[NUM_ENVIRONMENT_VARIABLES][ENVIRONMENT_VALUE_LEN];
    char * environment_str_value_pointers[NUM_ENVIRONMENT_VARIABLES];
} agent_task_t;

//Currently we can hold ten tasks
#define NUM_TASKS 10
static agent_task_t tasks[NUM_TASKS];
static agent_task_t empty_task;
static uint8_t environment_idx = 0;

void agent_ping(void);

bool generic_string_encode_callback(pb_ostream_t *ostream, const pb_field_iter_t *field, void * const* args) {
    //In this let's just set an ID and a name
    if (ostream != NULL) {
        if(!pb_encode_tag_for_field(ostream, field))
            return false;

        return pb_encode_string(ostream, (char*)*args, strlen((char*)*args));
    }

    return true;
}

bool wasm_binary_decode_callback(pb_istream_t *istream, const pb_field_iter_t *field, void** arg) {
    agent_task_t* t = *arg;

    //Allocate memory for the substream
    t->wasm_binary_length = istream->bytes_left;
    t->wasm_binary = agent_port_malloc(istream->bytes_left);
    if(!(t->wasm_binary)) {
        agent_port_print("Error allocating memory for stream");
        return false;
    }    

    //read the substream
    if(!pb_read(istream, t->wasm_binary, istream->bytes_left))
        return false;

    return true;
}

bool generic_string_decode_callback(pb_istream_t *istream, const pb_field_iter_t *field, void** arg) {
    //get my destination pointer
    char* dest = (char*)*arg;

    //read the substream
    if(!pb_read(istream, dest, istream->bytes_left))
        return false;

    return true;
}

bool environment_decode_callback(pb_istream_t *istream, const pb_field_iter_t *field, void** arg) {
    
    if(environment_idx <= NUM_ENVIRONMENT_VARIABLES - 1) {
        agent_task_t* t = *arg;

        //This should get called once for every environment variable
        ContainerInfo_WASMInfo_EnvironmentVariable e = ContainerInfo_WASMInfo_EnvironmentVariable_init_zero;

        //Setup the decoding callback
        e.key.funcs.decode = &generic_string_decode_callback;
        e.key.arg = t->environment_keys[environment_idx];
        e.str_value.funcs.decode = &generic_string_decode_callback;
        e.str_value.arg = t->environment_str_values[environment_idx];
        
        bool r = pb_decode(istream, ContainerInfo_WASMInfo_EnvironmentVariable_fields, &e);

        //Copy the one outside of the callback
        if (e.has_value) {
            t->environment_values[environment_idx] = e.value;
        } else {
            t->environment_values[environment_idx] = 0;
        }

        environment_idx++;
        
        return r;
    } else {
        //just decode it and put it nowhere
        ContainerInfo_WASMInfo_EnvironmentVariable e = ContainerInfo_WASMInfo_EnvironmentVariable_init_zero;
        return pb_decode(istream, ContainerInfo_WASMInfo_EnvironmentVariable_fields, &e);
    }
}



void construct_resource(pb_ostream_t* ostream, const char* name, Value_Type type, void* value, bool shared) {
    //For each resource 1) Initiate a resource message, 2) encode a submessage to initiate the resource callback
    Resource r  = Resource_init_zero;

    //Name
    r.name.funcs.encode = &generic_string_encode_callback;
    r.name.arg = (char*)name;

    //Type
    r.type = type;

    //Value
    switch(type) {
    case Value_Type_SCALAR:
        r.has_scalar = true;
        r.scalar.value = *(float*)value; 
        break;
    case Value_Type_TEXT:
        r.has_text = true;
        r.text.value.funcs.encode = &generic_string_encode_callback;
        r.text.value.arg = (char*)value;
        break;
    case Value_Type_DEVICE:
        r.has_device = true;
        r.device.device.funcs.encode = &generic_string_encode_callback;
        r.device.device.arg = (char*)value;
        break;
    case Value_Type_RANGES:
        r.has_ranges = true;
        break;
    case Value_Type_SET:
        r.has_set = true;
        break;
    }

    //shared
    r.shared = shared;

    pb_encode_submessage(ostream, Resource_fields, &r);
}

void construct_attribute(pb_ostream_t* ostream, const char* name, Value_Type type, void* value) {
    //For each resource 1) Initiate a resource message, 2) encode a submessage to initiate the resource callback
    Attribute a  = Attribute_init_zero;

    //Name
    a.name.funcs.encode = &generic_string_encode_callback;
    a.name.arg = (char*)name;

    //Type
    a.type = type;

    //Value
    switch(type) {
    case Value_Type_SCALAR:
        a.has_scalar = true;
        a.scalar.value = *(float*)value; 
        break;
    case Value_Type_TEXT:
        a.has_text = true;
        a.text.value.funcs.encode = &generic_string_encode_callback;
        a.text.value.arg = (char*)value;
        break;
    case Value_Type_RANGES:
        a.has_ranges = true;
        break;
    case Value_Type_SET:
        a.has_set = true;
        a.set.item.funcs.encode = &generic_string_encode_callback;
        a.set.item.arg = (char*)value;
        break;
    case Value_Type_DEVICE:
        break;
    }

    pb_encode_submessage(ostream, Attribute_fields, &a);
}



bool AgentInfo_attributes_callback(pb_ostream_t *ostream, const pb_field_iter_t *field, void * const* args) {
    //In this let's just set an ID and a name
    if (ostream != NULL) {

        if(!pb_encode_tag_for_field(ostream, field))
            return false;
        construct_attribute(ostream, "OS", Value_Type_TEXT, (char*)agent_port_get_os());

        if(!pb_encode_tag_for_field(ostream, field))
            return false;
        construct_attribute(ostream, "executors", Value_Type_SET, "WASM");

        return true;
    }

    return true;
}

TaskInfo construct_TaskInfo(agent_task_t* task) {
    TaskInfo t  = TaskInfo_init_zero;

    //Name
    t.name.funcs.encode = &generic_string_encode_callback;
    t.name.arg = task->task_name;
    t.framework.name.funcs.encode = &generic_string_encode_callback;
    t.framework.name.arg = task->framework_name;

    //ID
    t.task_id.funcs.encode = &generic_string_encode_callback;
    t.task_id.arg = task->task_id;
    t.framework.framework_id.funcs.encode = &generic_string_encode_callback;
    t.framework.framework_id.arg = task->framework_id;

    //ID
    t.agent_id.funcs.encode = &generic_string_encode_callback;
    t.agent_id.arg = (char*)agent_port_get_agent_id();

    //State
    t.has_state = true;
    t.state = task->state;

    //Error message
    if(task->error_message) {
        t.error_message.funcs.encode = &generic_string_encode_callback;
        t.error_message.arg = task->error_message;
    }

    //Container
    t.container.type = ContainerInfo_Type_WASM;

    return t;

}

bool TaskInfo_callback(pb_ostream_t *ostream, const pb_field_iter_t *field, void * const* args) {
    //In this let's just set an ID and a name
    if (ostream != NULL) {
        //First encode the running task

        for(uint8_t i = 0; i < NUM_TASKS; i++) {
            if(strlen(tasks[i].task_id) > 0) {
                if(!pb_encode_tag_for_field(ostream, field))
                    return false;
                
                TaskInfo t = construct_TaskInfo(&tasks[i]);
                if(!pb_encode_submessage(ostream, TaskInfo_fields, &t))
                    return false;
            }
        }
        
        return true;
    } 

    return false;
}

bool AgentInfo_resources_callback(pb_ostream_t *ostream, const pb_field_iter_t *field, void * const* args) {
    //In this let's just set an ID and a name
    if (ostream != NULL) {

        if(!pb_encode_tag_for_field(ostream, field))
            return false;
        float m = agent_port_get_free_memory();
        construct_resource(ostream, "mem", Value_Type_SCALAR, &m, false);

        if(!pb_encode_tag_for_field(ostream, field))
            return false;
        float c = agent_port_get_free_cpu();
        construct_resource(ostream, "cpus", Value_Type_SCALAR, &c, false);

        for(uint8_t i = 0; true; i++) {
           agent_device_t d;
           bool valid = agent_port_get_device(i, &d);
           if(!valid) {
               break;
           } else {
                if(!pb_encode_tag_for_field(ostream, field))
                    return false;
                construct_resource(ostream, d.name, Value_Type_DEVICE, d.reference, true);
           }
        }

        return true;
    }

    return true;
}

void agent_response_cb(uint8_t return_code, uint8_t* buf, uint32_t len) {
    agent_port_print("Got coap response with return code %d and length %d\n", return_code, len);

    //parse the coap response
    if(return_code == RESPONSE_CODE_CONTENT || return_code == RESPONSE_CODE_VALID) {

        //Init wrapper
        WrapperMessage wrapper = WrapperMessage_init_zero;

        //find an empty slot in the task wrapper
        agent_task_t* new_task = NULL;
        for(uint8_t i = 0; i < NUM_TASKS; i++) {
            if(tasks[i].task_id[0] == 0) {
                agent_port_print("Found empty task slot %d\n", i);
                new_task = &tasks[i];
                break;
            }
        }

        if(new_task == NULL) {
            //just get the first completed task and overwrite it
            for(uint8_t i = 0; i < NUM_TASKS; i++) {
                if(tasks[i].state != TaskInfo_TaskState_RUNNING) {
                    agent_port_print("Found empty task slot %d\n", i);
                    new_task = &tasks[i];
                    break;
                }
            }
        }

        //we have ten running tasks and just can't take anymore
        if(new_task == NULL) {
            agent_port_print("No empty task slot - setting task data to throw-away task\n");
            new_task = &empty_task;
        }



        //setup wrapper decoding for run_task
        wrapper.pong.run_task.task.task_id.funcs.decode = &generic_string_decode_callback;
        wrapper.pong.run_task.task.task_id.arg = new_task->task_id;
        wrapper.pong.run_task.task.name.funcs.decode = &generic_string_decode_callback;
        wrapper.pong.run_task.task.name.arg = new_task->task_name;
        wrapper.pong.run_task.task.framework.name.funcs.decode = &generic_string_decode_callback;
        wrapper.pong.run_task.task.framework.name.arg = new_task->framework_name;
        wrapper.pong.run_task.task.framework.framework_id.funcs.decode = &generic_string_decode_callback;
        wrapper.pong.run_task.task.framework.framework_id.arg = new_task->framework_id;

        //For the alloc decode you have to do this indirect pointer thing I think...maybe there is a better way, but it works?
        wrapper.pong.run_task.task.container.wasm.wasm_binary.funcs.decode = &wasm_binary_decode_callback;
        wrapper.pong.run_task.task.container.wasm.wasm_binary.arg = new_task;

        // Okay we needed a special decoder for the environment variables
        environment_idx = 0;
        wrapper.pong.run_task.task.container.wasm.environment.funcs.decode = &environment_decode_callback;
        wrapper.pong.run_task.task.container.wasm.environment.arg = new_task;

        //get the kill task id
        char kill_task_id[TASK_ID_LEN] = {0};
        wrapper.pong.kill_task.task_id.funcs.decode = &generic_string_decode_callback;
        wrapper.pong.kill_task.task_id.arg = kill_task_id;

        pb_istream_t stream = pb_istream_from_buffer(buf, len);
        bool r = pb_decode(&stream, WrapperMessage_fields, &wrapper);


        if(wrapper.type == WrapperMessage_Type_PONG) {

            if(wrapper.pong.has_run_task) {
                agent_port_print("Got Pong Message with run task request\n");
                agent_port_print("\nTask Info:\n");
                agent_port_print("\tTask ID: %s\n",new_task->task_id);
                agent_port_print("\tTask Name: %s\n",new_task->task_name);
                agent_port_print("\tFramework ID: %s\n",new_task->framework_id);
                agent_port_print("\tFramework Name: %s\n",new_task->framework_name);
                agent_port_print("\tWASM binary length: %d\n",new_task->wasm_binary_length);
                for(uint8_t i = 0; i < NUM_ENVIRONMENT_VARIABLES; i++) {
                    if(strlen(new_task->environment_keys[i]) > 0) {
                       agent_port_print("\tEnvironment Variable %d: %s = %d | %s\n",i,new_task->environment_keys[i],
                                        new_task->environment_values[i],new_task->environment_str_values[i]); 
                    } else {
                        break;
                    }
                }
                
                if(wrapper.pong.run_task.task.container.type == ContainerInfo_Type_WASM &&
                        wrapper.pong.run_task.task.container.has_wasm == true) {
                    // Is something already running
                    if(agent_port_can_run_task()) {
                        //Is the task valid?
                        if(new_task->wasm_binary != NULL && strlen(new_task->task_id) > 0) {
                            // Run the task
                            agent_port_print("Running WASM Task!\n");
                            
                            //Setup the environment keys
                            for(uint8_t i = 0; i < NUM_ENVIRONMENT_VARIABLES; i++) {
                                new_task->environment_key_pointers[i] = new_task->environment_keys[i];
                                new_task->environment_str_value_pointers[i] = new_task->environment_str_values[i];
                            }

                            //Start the WASM task
                            bool success = agent_port_run_wasm_task(new_task->wasm_binary,
                                                        new_task->wasm_binary_length,
                                                        new_task->environment_key_pointers,
                                                        new_task->environment_values,
                                                        new_task->environment_str_value_pointers,
                                                        NUM_ENVIRONMENT_VARIABLES,
                                                        new_task->task_id);

                            if(success) {
                                // Set the task state in the task struct
                                new_task->state = TaskInfo_TaskState_RUNNING;

                                //Send a ping immediately
                                agent_ping();
                            } else {
                                agent_port_print("RUNNING WASM task failed\n");
                                new_task->state = TaskInfo_TaskState_ERRORED;
                                new_task->error_message = "Failed to start WASM";
                                //Send a ping immediately
                                agent_ping();
                            }
                        } else {
                            agent_port_print("WASM task invalid\n");
                            new_task->state = TaskInfo_TaskState_ERRORED;
                            new_task->error_message = "Task Protobuf Invalid";
                            //Send a ping immediately
                            agent_ping();
                        }
                    } else {
                        // Set the task state and error message in the task struct
                        agent_port_print("Can't run task - task already running\n");
                        new_task->state = TaskInfo_TaskState_ERRORED;
                        new_task->error_message = "Insufficient Resources";
                        //Send a ping immediately
                        agent_ping();
                    }
                } else {
                    // Set the task state and error message in the task struct
                    agent_port_print("Got docker task\n");
                    new_task->state = TaskInfo_TaskState_ERRORED;
                    new_task->error_message = "Invalid Executor";
                    //Send a ping immediately
                    agent_ping();
                }


            } else if (wrapper.pong.has_kill_task) {
                agent_port_print("Got Pong Message with kill task request\n");

                agent_port_print("ID of task to kill: %s\n", kill_task_id);

                //loop through tasks to find wasm task to kill
                bool found_task = false;
                for(uint8_t i = 0; i < NUM_TASKS; i++) {
                    if(strncmp(tasks[i].task_id, kill_task_id, TASK_ID_LEN) == 0) {
                        found_task = true;
                        agent_port_print("Task ID to kill matches running task. Killing\n");
                        agent_port_kill_wasm_task(kill_task_id);
                    }
                }

                if(found_task == false) {
                    agent_port_print("Task ID to kill does not match a running task.\n");
                }
            } else {
                agent_port_print("Got Pong Message\n");
            }
        } else if (wrapper.type == WrapperMessage_Type_RUN_TASK) {
            agent_port_print("Got Run Task Message\n");
        } else if (wrapper.type == WrapperMessage_Type_KILL_TASK) {
            agent_port_print("Got Kill Task Message\n");
        } else {
            agent_port_print("Got unknown message\n");
        }

    } else {
        agent_port_print("Not a valid response code - not parsing\n");
    }
}

void agent_init(const char* master) {
    // Note the domain
    master_domain = master;

    //zero out the tasks array
    for(uint8_t i = 0; i < NUM_TASKS; i++) {
        tasks[i].task_id[0] = (char)0;
    }

    // Register the coap receive callback
    agent_port_register_coap_receive_cb(&agent_response_cb);
}

void agent_ping(void) {
    agent_port_print("Pinging\n");

    //Construct nanopb message
    WrapperMessage wrapper  = WrapperMessage_init_zero;

    //fill out the fields that are not callbacks

    // AgentInfo
    wrapper.type = WrapperMessage_Type_PING;
    wrapper.has_ping = true;
    wrapper.ping.agent.has_ping_rate = true;
    wrapper.ping.agent.ping_rate = ping_rate * 1000;
    wrapper.ping.agent.id.funcs.encode = &generic_string_encode_callback;
    wrapper.ping.agent.id.arg = (char*)agent_port_get_agent_id();
    wrapper.ping.agent.name.funcs.encode = &generic_string_encode_callback;
    wrapper.ping.agent.name.arg = (char*)agent_port_get_agent_name();
    wrapper.ping.agent.resources.funcs.encode = &AgentInfo_resources_callback;
    wrapper.ping.agent.attributes.funcs.encode = &AgentInfo_attributes_callback;

    // TaskInfo
    wrapper.ping.tasks.funcs.encode = &TaskInfo_callback;

    //update the state of running tasks
    for(uint8_t i = 0; i < NUM_TASKS; i++) {
        if(tasks[i].task_id[0] != 0) {
            task_state_t s = agent_port_get_wasm_task_state(tasks[i].task_id, &(tasks[i].error_message));

            if(s == RUNNING) {
                tasks[i].state = TaskInfo_TaskState_RUNNING;
            } else if (s == COMPLETED) {
                tasks[i].state = TaskInfo_TaskState_COMPLETED;
            } else if (s == STARTING) {
                tasks[i].state = TaskInfo_TaskState_STARTING;
            } else if(s == ERRORED) {
                tasks[i].state = TaskInfo_TaskState_ERRORED;
                agent_port_print("Got error message: %s",tasks[i].error_message);
            }
        }
    }

    //Get the size of the payload
    pb_ostream_t sizestream = {0};
    pb_encode(&sizestream, WrapperMessage_fields, &wrapper);
    size_t wrapper_message_buffer_size = sizestream.bytes_written;

    //allocate and pack the payload
    uint8_t* wrapper_message_buffer = (uint8_t*)agent_port_malloc(wrapper_message_buffer_size);
    if(!wrapper_message_buffer) {
        agent_port_print("Error - no memory to build payload");
        return;
    }
    pb_ostream_t bufstream = pb_ostream_from_buffer(wrapper_message_buffer, wrapper_message_buffer_size);

    //now when we call encode we will hit all the callbacks
    pb_encode(&bufstream, WrapperMessage_fields, &wrapper);

    //send ping through port layer
    agent_port_print("Sending packet\n");
    agent_port_coap_send(master_domain, 5683, "ping", wrapper_message_buffer, wrapper_message_buffer_size);
    agent_port_print("Done sending packet\n");

    agent_port_free(wrapper_message_buffer);
}

void agent_start(uint32_t ping_rate_s) {
    agent_port_start_timer_repeated(ping_rate_s*1000, &agent_ping);
    ping_rate = ping_rate_s;
}

void agent_stop() {
    agent_port_stop_timer_repeated();
}
