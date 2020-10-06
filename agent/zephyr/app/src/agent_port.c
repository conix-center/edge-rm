#include <logging/log.h>
LOG_MODULE_REGISTER(agent_port, LOG_LEVEL_DBG);

#include <zephyr.h>
#include "agent_port.h"
#include "coap_help.h"
#include "config.h"

agent_port_timer_cb local_cb;
agent_port_coap_receive_cb recv_cb;

void agent_work_handler(struct k_work *work)
{
    local_cb();
}

K_WORK_DEFINE(agent_timer_work, agent_work_handler);

void agent_timer_function(struct k_timer *t_struct)
{
    k_work_submit(&agent_timer_work);
}

K_TIMER_DEFINE(agent_timer, agent_timer_function, NULL);

void agent_port_start_timer_repeated(uint32_t timer_rate_ms, agent_port_timer_cb cb) {
    local_cb = cb;
    k_timer_start(&agent_timer, K_MSEC(5000), K_MSEC(timer_rate_ms));
}

void agent_port_stop_timer_repeated() {
    k_timer_stop(&agent_timer);
}

void agent_port_register_coap_receive_cb(agent_port_coap_receive_cb cb) {
   recv_cb = cb;
}

void agent_port_coap_send(const char* destination, uint8_t* payload, uint32_t len) {
   //Send the packet
   send_coap_request(payload, len);

   //Allocate date for the response
   uint8_t ret_code = 0;
   uint32_t ret_len = 0;
   uint8_t* recv_payload = (uint8_t *)k_malloc(MAX_COAP_MSG_LEN);
   if (!recv_payload) {
       return -ENOMEM;
   }

   //Wait for the response
   int ret = process_coap_reply(&ret_code, recv_payload, &ret_len);
   if (ret < 0) {
      LOG_ERR("Got error return code %d", ret);
      k_free(recv_payload);
      return;
   }

   //Call the return callback
   recv_cb(ret_code, recv_payload, ret_len);

   //free and exit
   k_free(recv_payload);
}

void agent_port_print(const char * fmt, ...) {
   va_list args;
   va_start(args, fmt);
   vprintk(fmt, args);
   va_end(args);
}

void* agent_port_malloc(size_t size) {
   return k_malloc(size);
}

void agent_port_free(void* pt) {
   k_free(pt);
}

//Resources
float agent_port_get_free_memory() {
   return (float)CONFIG_HEAP_MEM_POOL_SIZE;
}

float agent_port_get_free_cpu() {
   return 1.0;
}

const char* agent_port_get_agent_id() {
   return AGENT_ID;
}

const char* agent_port_get_agent_name() {
   return AGENT_NAME;
}

const char* agent_port_get_os() {
   return "zephyr";
}

bool agent_port_get_device(uint8_t device_number, agent_device_t* device) {
   switch(device_number) {
   case 0:
        device->name = "temperature_sensor";
        device->reference = "temp";
        return true;
        break;
   case 1:
        device->name = "humidity_sensor";
        device->reference = "hum";
        return true;
        break;
   }

   return false; 
}

