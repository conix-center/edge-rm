#ifndef COAP_HELPER_H
#define COAP_HELPER_H

#include <zephyr.h>
#include <stdint.h>

int start_coap_client(void);
int send_coap_request(uint8_t* data, uint32_t len);
int process_coap_reply(uint8_t* data, uint32_t* len);

#endif
