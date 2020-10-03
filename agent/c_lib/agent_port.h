#ifndef AGENT_PORT_H
#define AGENT_PORT_H

#include <stdio.h>
#include <stdint.h>

typedef void (*agent_port_timer_cb)(); 
typedef void (*agent_port_coap_receive_cb)(uint8_t*, uint32_t); 

void agent_port_start_timer_repeated(uint32_t timer_rate_ms, agent_port_timer_cb cb);
void agent_port_stop_timer_repeated();
void agent_port_register_coap_receive_cb(agent_port_coap_receive_cb cb);
void agent_port_coap_send(const char* destination, uint8_t* payload, uint32_t len);
void agent_port_print(const char* str);

#endif
