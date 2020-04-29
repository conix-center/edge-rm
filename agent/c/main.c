#include "hal/edge_hal.h"
#include "messages.pb.h"
#include "pb_common.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "string.h"
#include "stdio.h"
#include "nrf_gpio.h"

#include "permamote.h"

static uint8_t ipv4[] = {128,97,92,77};

//This callback responds to the slaveInfo
bool SlaveInfo_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field) {
    edge_hal_print("Encoding slave info");
    edge_hal_debug_led_on();

    if(ostream != NULL && field->tag == SlaveInfo_resources_tag) {
        //construct resource
        // Get the resources
        Resource* resources;
        resources = edge_hal_create_resources();

        //encode resources submessage for every resource returned in a loop

    } else if(ostream != NULL && field->tag == SlaveInfo_attributes_tag) {
        //same as above
    } else if(ostream != NULL && field->tag == SlaveInfo_id_tag) {
        // we might have to use a special callback fro the string here? unclear
    }

    return true;
}

bool RegisterSlaveMessage_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field) {
    edge_hal_print("Encoding register slave");
    edge_hal_debug_led_on();
    if(ostream != NULL && field->tag == RegisterSlaveMessage_slave_tag) {

    } 
    return true;
}

void coap_callback(uint8_t upper_code, uint8_t lower_code) {
   edge_hal_print("Got coap callback");
}

int main(void) {

    // Initialize the HAL
    uint8_t ret = edge_hal_initialize();

    edge_hal_print("Initialized");

    // Register the slave
    uint8_t buffer[256];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    RegisterSlaveMessage register_slave = RegisterSlaveMessage_init_zero;

    // Go ahead and encode the slave info, all of the fields will be handled
    // by callbacks
    uint8_t status = pb_encode(&stream, RegisterSlaveMessage_fields, &register_slave);

    while(1) {
        edge_hal_process();
    }

    //Send the RegisterSlave message
    edge_hal_coap_send(ipv4, "RegisterSlave", buffer, stream.bytes_written, coap_callback);
}
