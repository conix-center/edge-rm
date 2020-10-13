#include <logging/log.h>
LOG_MODULE_REGISTER(agent_port, LOG_LEVEL_DBG);

#include <zephyr.h>
#include "agent_port.h"
#include "coap_help.h"
#include "config.h"
#include "agent_wasm_runtime.h"

agent_port_timer_cb local_cb;
agent_port_coap_receive_cb recv_cb;
k_tid_t wasm_thread;
static bool running_task = false;

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

void agent_port_coap_send(const char* destination, uint16_t port, char* path, uint8_t* payload, uint32_t len) {
   //Clear the receive buffer
   uint8_t ret_code = 0;
   uint16_t ret_len = 0;

   int ret = 1;
   for(uint8_t i = 0; i < 100 && ret >= 0; i++) {
      ret = process_coap_reply(100, &ret_code, NULL, &ret_len);
   } 

   //Send the packet
   send_coap_request(destination, port, path, payload, len);

   //Allocate date for the response
   uint8_t* recv_payload = (uint8_t *)k_malloc(MAX_COAP_MSG_LEN);
   if (!recv_payload) {
      LOG_ERR("Error allocating memory");
      return;
   }

   //Wait for the response
   ret = process_coap_reply(10000, &ret_code, recv_payload, &ret_len);
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

bool agent_port_can_run_task(void) {
   task_state_t s = agent_port_get_wasm_task_state(NULL);

   if(running_task == true && s != COMPLETED && s != ERRORED) {
      return false;
   } else {
      running_task = false;
      return true;
   }
}

//Resources
float agent_port_get_free_memory() {
   return (float)CONFIG_HEAP_MEM_POOL_SIZE;
}

float agent_port_get_free_cpu() {
   if(agent_port_can_run_task()){
      return 1.0;
   } else {
      return 0;
   }
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
        device->reference = "humidity";
        return true;
        break;
   case 2:
        device->name = "pressure_sensor";
        device->reference = "press";
        return true;
        break;
   }

   return false; 
}

bool agent_port_run_wasm_task(uint8_t* wasm_binary, 
                            uint32_t wasm_binary_length,
                            char* environment_keys[],
                            int32_t environment_int_values[],
                            char* environment_str_values[],
                            uint8_t num_environment_variables) {

   //start the module, save the thread
   running_task = true;
   wasm_thread = run_wasm_module(wasm_binary, wasm_binary_length, environment_keys, environment_int_values, environment_str_values, num_environment_variables);

   return true;
}

task_state_t agent_port_get_wasm_task_state(char** error_message) {

   uint32_t state =  wasm_thread->base.thread_state;

   if(state & _THREAD_DEAD || state & _THREAD_ABORTING) {
      //check for errors 
      if(check_wasm_errored()) {
         if(error_message != NULL) {
            *error_message = get_wasm_error_message();
         }
         return ERRORED;
      } else {
         return COMPLETED;
      }
      running_task = false;
   } else if(state & _THREAD_SUSPENDED || state & _THREAD_QUEUED || state & _THREAD_PENDING) {
      return RUNNING;
   } else if (state & _THREAD_PRESTART) {
      return STARTING;
   } else {
      return RUNNING;
   }
}

bool agent_port_kill_wasm_task(void) {
   k_thread_abort(wasm_thread);
   cleanup_wasm_module();
   return true;
}
