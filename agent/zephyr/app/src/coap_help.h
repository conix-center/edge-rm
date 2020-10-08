#ifndef COAP_HELPER_H
#define COAP_HELPER_H

#include <zephyr.h>
#include <stdint.h>

#define PEER_PORT 5683
#define MAX_COAP_MSG_LEN 2048

int send_coap_request(uint8_t* data, uint32_t len);
int process_coap_reply(uint8_t* return_code, uint8_t* data, uint16_t* len);

#endif
