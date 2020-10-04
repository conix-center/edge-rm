#ifndef AGENT_LIBRARY_H
#define AGENT_LIBRARY_H

#include <stdio.h>
#include <stdint.h>

// Initialize the agent with a string that is a valid IPV4, IPV6 or domain name
// This is passed directly to the COAP layer in the port
void agent_init(const char* master);

// This starts the agent process to ping at a set rate
void agent_start(uint32_t ping_rate_s);

// This stops the agent process
void agent_stop();

#endif
