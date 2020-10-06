#include "agent_library.h"
#include "agent_port.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "pb_common.h"
#include "messages.pb.h"
#include <string.h>

const char* master_domain;
static uint32_t ping_rate;

bool generic_string_encode_callback(pb_ostream_t *ostream, const pb_field_iter_t *field, void * const* args) {
    //In this let's just set an ID and a name
    if (ostream != NULL) {
        if(!pb_encode_tag_for_field(ostream, field))
            return false;

        return pb_encode_string(ostream, (char*)*args, strlen((char*)*args));
    }

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
        construct_resource(ostream, "cpu", Value_Type_SCALAR, &c, false);

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

//nanopb encoding callbacks
bool PingAgentMessage_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_iter_t *field) {
    // For now we aren't going to set any tasks so just leave this
}

void agent_response_cb(uint8_t return_code, uint8_t* buf, uint32_t len) {
    agent_port_print("Got coap response with return code %d and length %d\n", return_code, len);
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
    wrapper.msg.ping.agent.has_ping_rate = true;
    wrapper.msg.ping.agent.ping_rate = ping_rate;
    wrapper.msg.ping.agent.id.funcs.encode = &generic_string_encode_callback;
    wrapper.msg.ping.agent.id.arg = agent_port_get_agent_id();
    wrapper.msg.ping.agent.name.funcs.encode = &generic_string_encode_callback;
    wrapper.msg.ping.agent.name.arg = agent_port_get_agent_name();

    wrapper.msg.ping.agent.resources.funcs.encode = &AgentInfo_resources_callback;
    wrapper.msg.ping.agent.attributes.funcs.encode = &AgentInfo_attributes_callback;

    wrapper.which_msg = WrapperMessage_ping_tag;

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
