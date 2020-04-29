#ifndef EDGE_HAL
#define EDGE_HAL

#include <stdint.h>

// Set the first argument to none so that we can have a variable length array
// and the first argument is none
typedef enum {
    NONE = 0,
    SCALAR,
    RANGES,
    SET,
    TEXT
} ResourceType;

// 
typedef struct {
    ResourceType type;
    void* value;
} Resources;

// Returns 0 for success 1 for failure
// Generic method for initializing anything necessary for other functions on hardware
uint8_t edge_hal_initialize(void);

//Method for doing periodic hardware specific processing
void edge_hal_process(void);

//Method for going into a low-power state
void edge_hal_sleep(void);

void edge_hal_print(const char* fmt);
void edge_hal_debug_led_on(void);
void edge_hal_debug_led_off(void);

// Sends COAP message to the requests URI, and calls the callback when it is acknowledged
typedef void (*callback)(uint8_t upper_response_code, uint8_t lower_response_code);
uint8_t edge_hal_coap_send(uint8_t* ipv4_address, const char* path, uint8_t* payload, uint8_t payload_len, callback acknowledge_callback);

// Mallocs and fills in an array of resources that the hardware offers
// Array should be zero padded (ResourceType NONE)
Resources* edge_hal_create_resources(void);

// Updates the available resoruces
uint8_t edge_hal_update_resources(void);

#endif
