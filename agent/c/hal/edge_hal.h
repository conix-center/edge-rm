#ifndef EDGE_HAL
#define EDGE_HAL

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
} Resource;

// Returns 0 for success 1 for failure
// Generic method for initializing anything necessary for other functions on hardware
uint8_t egde_hal_initialize(void);

// Sends COAP message to the requests URI, and calls the callback when it is acknowledged
typedef void (*callback)(void);
uint8_t edge_hal_coap_send(char* uri, uint8_t uri_len, uint8_t* payload, uint8_t payload_len, callback acknowledge_callback);

// Mallocs and fills in an array of resources that the hardware offers
// Array should be zero padded (ResourceType NONE)
Resource* edge_hal_create_resources(void)

// Updates the available resoruces
uint8_t edge_hal_update_resources(void)

#endif
