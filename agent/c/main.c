
#include "hal/edge_hal.h"
#include "pb_common.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "messages.pb.h"
#include "string.h"

const char* master_uri = "coap://master.com"

//This callback responds to the slaveInfo
bool SlaveInfo_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_iter_t *field) {
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
}

int main(void) {

    // Initialize the HAL
    uint8_t ret = edge_hal_initialize();


    // Register the slave
    uint8_t buffer[256];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    SlaveInfo slave_info = SlaveInfo_init_zero;

    // Go ahead and encode the slave info, all of the fields will be handled
    // by callbacks
    uint8_t status = pb_encode(&stream, SlaveInfo_fields, &slave_info);

    //Send the RegisterSlave message
    edge_hal_coap_send(master_uri, strlen(master_uri),buffer, stream.bytes_written, callback);

    // Ping periodically (assume ack is the pong)
}
