#include "agent_library.h"
#include "agent_port.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "pb_common.h"
#include "messages.pb.h"
#include <string.h>

const char* master_domain;

void agent_response_cb(uint8_t return_code, uint8_t* buf, uint32_t len) {
    agent_port_print("Got coap response with return code %d and lenght %d", return_code, len);
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

    //Construct nanopb message
    const char* test = "This is my test string";

    //send ping through port layer
    agent_port_print("Sending packet\n");
    agent_port_coap_send(master_domain, test, strlen(test));
    agent_port_print("Done sending packet\n");
}

void agent_start(uint32_t ping_rate_s) {
    agent_port_start_timer_repeated(ping_rate_s*1000, &agent_ping);
}

void agent_stop() {
    agent_port_stop_timer_repeated();
}
