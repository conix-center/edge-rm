#include "agent_library.h"
#include "agent_port.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "pb_common.h"
#include "messages.pb.h"
#include <string.h>

const char* master_domain;
static uint32_t ping_rate;

//nanopb encoding callbacks
bool AgentInfo_callback(pb_ostream_t *ostream, const pb_field_iter_t *field, void * const* args) {
    //In this let's just set an ID and a name
    if (ostream != NULL && field->tag == AgentInfo_id_tag) {
        // Fill out the agent ID
        if(!pb_encode_tag_for_field(ostream, field)) {
            return false;
        }

        return pb_encode_string(ostream, "00116688", strlen("00116688"));

    } else if (ostream != NULL && field->tag == AgentInfo_name_tag) {
        // Fill out the agent name
        if(!pb_encode_tag_for_field(ostream, field)) {
            return false;
        }

        return pb_encode_string(ostream, "EmbeddedAgent", strlen("EmbeddedAgent"));
    }

    return true;
}

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
    wrapper.msg.ping.agent.id.funcs.encode = &AgentInfo_callback;
    wrapper.msg.ping.agent.name.funcs.encode = &AgentInfo_callback;
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
