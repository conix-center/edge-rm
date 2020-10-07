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
typedef struct _agent_task {
    char task_name[TASK_NAME_LEN];
    char task_id[TASK_ID_LEN];
    char* error_message;
    TaskInfo_TaskState state;
    uint8_t* wasm;
} agent_task_t;

agent_task_t new_task;
agent_task_t running_task;

void set_running_task_to_new_task(void) {
    //Does the current running task have some wasm allocated?
    if(running_task.wasm != NULL) {
        agent_port_free(running_task.wasm);
    } 

    //Overwrite the current running task with the new task
    running_task.error_message = new_task.error_message;
    running_task.wasm = new_task.wasm;
    running_task.state = TaskInfo_TaskState_RUNNING;
    memcpy(running_task.task_id, new_task.task_id, TASK_ID_LEN);
    memcpy(running_task.task_name, new_task.task_name, TASK_NAME_LEN);

    //Clear out the new_task info
    new_task.error_message = "";
    memset(new_task.task_id, 0, TASK_ID_LEN);
    memset(new_task.task_name, 0, TASK_NAME_LEN);
    new_task.wasm = NULL;
}

bool generic_string_encode_callback(pb_ostream_t *ostream, const pb_field_iter_t *field, void * const* args) {
    //In this let's just set an ID and a name
    if (ostream != NULL) {
        if(!pb_encode_tag_for_field(ostream, field))
            return false;

        return pb_encode_string(ostream, (char*)*args, strlen((char*)*args));
    }

    return true;
}

bool generic_string_alloc_decode_callback(pb_istream_t *istream, const pb_field_iter_t *field, void** arg) {
    //get my destination pointer
    char* dest = (char*)*arg;

    //Allocate memory for the substream
    dest = agent_port_malloc(istream->bytes_left);
    if(!dest) {
        agent_port_print("Error allocating memory for stream");
        return false;
    }    

    //read the substream
    if(!pb_read(istream, dest, istream->bytes_left))
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

void construct_resource(pb_ostream_t* ostream, const char* name, Value_Type type, void* value, bool shared) {
    //For each resource 1) Initiate a resource message, 2) encode a submessage to initiate the resource callback
    Resource r  = Resource_init_zero;

    //Name
    r.name.funcs.encode = &generic_string_encode_callback;
    r.name.arg = name;

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
    a.name.arg = name;

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
    }

    pb_encode_submessage(ostream, Attribute_fields, &a);
}



bool AgentInfo_attributes_callback(pb_ostream_t *ostream, const pb_field_iter_t *field, void * const* args) {
    //In this let's just set an ID and a name
    if (ostream != NULL) {

        if(!pb_encode_tag_for_field(ostream, field))
            return false;
        construct_attribute(ostream, "OS", Value_Type_TEXT, agent_port_get_os());

        if(!pb_encode_tag_for_field(ostream, field))
            return false;
        construct_attribute(ostream, "executors", Value_Type_SET, "WASM");

        return true;
    }
}

TaskInfo construct_TaskInfo(agent_task_t* task) {
    TaskInfo t  = TaskInfo_init_zero;

    //Name
    t.name.funcs.encode = &generic_string_encode_callback;
    t.name.arg = running_task.task_name;

    //ID
    t.task_id.funcs.encode = &generic_string_encode_callback;
    t.task_id.arg = running_task.task_id;

    //ID
    t.agent_id.funcs.encode = &generic_string_encode_callback;
    t.agent_id.arg = agent_port_get_agent_id();

    //State
    t.has_state = true;
    t.state = running_task.state;

    //Error message
    t.error_message.funcs.encode = &generic_string_encode_callback;
    t.error_message.arg = running_task.error_message;

    //Container
    t.container.type = ContainerInfo_Type_WASM;

    return t;

}

bool TaskInfo_callback(pb_ostream_t *ostream, const pb_field_iter_t *field, void * const* args) {
    //In this let's just set an ID and a name
    if (ostream != NULL) {
        //First encode the running task
        if(strlen(running_task.task_id) > 0) {
            if(!pb_encode_tag_for_field(ostream, field))
                return false;
            
            TaskInfo t = construct_TaskInfo(&running_task);
            if(!pb_encode_submessage(ostream, TaskInfo_fields, &t))
                return false;
        }

        //First encode the running task
        if(strlen(new_task.task_id) > 0) {
            if(!pb_encode_tag_for_field(ostream, field))
                return false;
            
            TaskInfo t = construct_TaskInfo(&new_task);
            if(!pb_encode_submessage(ostream, TaskInfo_fields, &t))
                return false;
        }
    } 
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
}

void agent_response_cb(uint8_t return_code, uint8_t* buf, uint32_t len) {
    agent_port_print("Got coap response with return code %d and length %d\n", return_code, len);

    //parse the coap response
    if(return_code == RESPONSE_CODE_CONTENT || return_code == RESPONSE_CODE_VALID) {

        //Init wrapper
        WrapperMessage wrapper = WrapperMessage_init_zero;

        //setup wrapper decoding for run_task
        wrapper.pong.run_task.task.task_id.funcs.decode = &generic_string_decode_callback;
        wrapper.pong.run_task.task.task_id.arg = new_task.task_id;
        wrapper.pong.run_task.task.name.funcs.decode = &generic_string_decode_callback;
        wrapper.pong.run_task.task.name.arg = new_task.task_name;
        //wrapper.pong.run_task.task.agent_id.funcs.decode = &generic_string_decode_callback;
        //wrapper.pong.run_task.task.agent_id.arg = new_task.agent_id;

        wrapper.pong.run_task.task.container.wasm.wasm_binary.funcs.decode = &generic_string_alloc_decode_callback;
        wrapper.pong.run_task.task.container.wasm.wasm_binary.arg = new_task.wasm;

        //wrapper.pong.run_task.task.framework.name.funcs.decode = &generic_string_decode_callback;
        //wrapper.pong.run_task.task.framework.name.arg = test;
        //wrapper.pong.run_task.task.framework.framework_id.funcs.decode = &generic_string_decode_callback;
        //wrapper.pong.run_task.task.framework.framework_id.arg = test;

        pb_istream_t stream = pb_istream_from_buffer(buf, len);
        bool r = pb_decode(&stream, WrapperMessage_fields, &wrapper);

        agent_port_print("Decode status %d\n",(int)r);
        agent_port_print("Task ID %s\n",new_task.task_id);
        agent_port_print("Task Name %s\n",new_task.task_name);


        if(wrapper.type == WrapperMessage_Type_PONG) {

            if(wrapper.pong.has_run_task) {
                agent_port_print("Got Pong Message with run task request\n");
                
                if(wrapper.pong.run_task.task.container.type == ContainerInfo_Type_WASM &&
                        wrapper.pong.run_task.task.container.has_wasm == true) {
                    // Is something already running
                    if(agent_port_can_run_task()) {
                        //Is the task valid?
                        if(new_task.wasm != NULL && strlen(new_task.task_id) > 0) {
                            // Run the task
                            agent_port_print("Running WASM Task!\n");
                            // Set the task state in the task struct
                        } else {
                            agent_port_print("WASM task invalid\n");
                            new_task.state = TaskInfo_TaskState_ERRORED;
                            new_task.error_message = "Invalid WASM task";
                        }
                    } else {
                        // Set the task state and error message in the task struct
                        agent_port_print("Can't run task - task already running\n");
                        new_task.state = TaskInfo_TaskState_ERRORED;
                        new_task.error_message = "Insufficient Resources";
                    }
                } else {
                    // Set the task state and error message in the task struct
                    agent_port_print("Got docker task\n");
                    new_task.state = TaskInfo_TaskState_ERRORED;
                    new_task.error_message = "Only WASM Executor";
                }


            } else if (wrapper.pong.has_kill_task) {
                agent_port_print("Got Pong Message with kill task request\n");
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

    // Register the coap receive callback
    agent_port_register_coap_receive_cb(&agent_response_cb);
}

void agent_ping(void) {
    agent_port_print("Pinging\n");

    //Get resources from agent

    // Construct an agent message
    //AgentInfo agent;
    //agent.ping_rate

    //Construct nanopb message
    WrapperMessage wrapper  = WrapperMessage_init_zero;

    //fill out the fields that are not callbacks

    // AgentInfo
    wrapper.type = WrapperMessage_Type_PING;
    wrapper.has_ping = true;
    wrapper.ping.agent.has_ping_rate = true;
    wrapper.ping.agent.ping_rate = ping_rate * 1000;
    wrapper.ping.agent.id.funcs.encode = &generic_string_encode_callback;
    wrapper.ping.agent.id.arg = agent_port_get_agent_id();
    wrapper.ping.agent.name.funcs.encode = &generic_string_encode_callback;
    wrapper.ping.agent.name.arg = agent_port_get_agent_name();
    wrapper.ping.agent.resources.funcs.encode = &AgentInfo_resources_callback;
    wrapper.ping.agent.attributes.funcs.encode = &AgentInfo_attributes_callback;

    // TaskInfo
    //wrapper.ping.tasks.funcs.encode = &TaskInfo_callback;

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
    agent_port_coap_send(master_domain, wrapper_message_buffer, wrapper_message_buffer_size);
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
