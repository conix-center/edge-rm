
#include <Particle.h>
#include "simple-coap.h"
#include "agent_port.h"

/*k_tid_t wasm_thread;
static bool running_task = false;*/

extern Coap coap;

agent_port_timer_cb local_cb;
uint8_t timer_flag = 0;

void timer_func(void) {
   timer_flag = 1;
}

Timer timer(100,timer_func);

void agent_port_start_timer_repeated(uint32_t timer_rate_ms, agent_port_timer_cb cb) {
   local_cb = cb;
   timer.changePeriod(timer_rate_ms);
   timer.start();
}

void agent_port_stop_timer_repeated() {
   timer.stop();
}

agent_port_coap_receive_cb recv_cb;
void agent_port_register_coap_receive_cb(agent_port_coap_receive_cb cb) {
   recv_cb = cb;
}

// CoAP client response callback
void response_callback(CoapPacket &packet, IPAddress ip, int port) {
    Serial.println("[Got Coap Response]");

    //call recv_cb
    recv_cb(packet.code, packet.payload, packet.payloadlen);
}

IPAddress IPfromString(const char* ipv4) {
	uint8_t d[5];
	uint8_t d_idx = 1;
	d[4] = strlen(ipv4);
	d[0] = 0;
	for(uint8_t i = 0; i < strlen(ipv4); i++) {
		if(ipv4[i] == '.') {
			d[d_idx] = i;
			d_idx++;
			if(d_idx == 4) {
				break;
			}
		}
	}

	if(d_idx != 4) {
		return IPAddress(0,0,0,0);
	}

	if(d[0] >= d[1] || d[1] >= d[2] || d[2] >= d[3] || d[3] >= d[4]) {
		return IPAddress(0,0,0,0);
	}

	char ip_str[4];
	uint8_t ip[4];
	for(uint8_t i = 0; i < 4; i++) {
		memset(ip_str,0,4);
		if(i == 0) {
		    memcpy(ip_str,ipv4 + d[i],d[i+1]-d[i]);
		} else {
		    memcpy(ip_str,ipv4 + d[i] + 1,d[i+1]-d[i]-1);
		}
		
		ip[i] = atoi(ip_str);
		if (ip[i] == 0 && strcmp(ip_str,"0") != 0) {
           return IPAddress(0,0,0,0);
		}
	}

    return IPAddress(ip[0],ip[1],ip[2],ip[3]);
}

void agent_port_coap_send(const char* destination, uint16_t port, char* path, uint8_t* payload, uint32_t len) {

   //setup the response callback
   coap.response(response_callback);
   
   //format the packet - we might have to take this and change it to be
   int msgid = coap.send(IPfromString(destination), 5683, path, COAP_CON, COAP_POST, NULL, 0, payload, len);
}

void agent_port_print(const char * fmt, ...) {
   va_list args;
   va_start(args, fmt);
   Serial.printf(fmt, args);
   va_end(args);
}

void* agent_port_malloc(size_t size) {
   return malloc(size);
}

void agent_port_free(void* pt) {
   free(pt);
}

bool agent_port_can_run_task(void) {
   /*task_state_t s = agent_port_get_wasm_task_state(NULL);

   if(running_task == true && s != COMPLETED && s != ERRORED) {
      return false;
   } else {
      running_task = false;
      return true;
   }*/
}

//Resources
float agent_port_get_free_memory() {
   return System.freeMemory();
}

float agent_port_get_free_cpu() {
   /*if(agent_port_can_run_task()){
      return 1.0;
   } else {
      return 0;
   }*/
}

const char* agent_port_get_agent_id() {
   return System.deviceID().c_str();
}

const char* agent_port_get_agent_name() {
   String s = "PowerWatch-";
   s.concat(System.deviceID());
   return s.c_str();
}

const char* agent_port_get_os() {
   String s = "particle";
   s.concat(System.version());
   return s.c_str();
}

bool agent_port_get_device(uint8_t device_number, agent_device_t* device) {
   switch(device_number) {
   case 0:
        device->name = (char*)"accelerometer-x";
        device->reference = (char*)"accelx";
        return true;
        break;
   case 1:
        device->name = (char*)"accelerometer-y";
        device->reference = (char*)"accely";
        return true;
        break;
   case 2:
        device->name = (char*)"accelerometer-z";
        device->reference = (char*)"accelz";
        return true;
        break;
   case 3:
        device->name = (char*)"has_electricity";
        device->reference = (char*)"power";
        return true;
        break;
   case 4:
        device->name = (char*)"voltage-rms";
        device->reference = (char*)"vrms";
        return true;
        break;
   case 5:
        device->name = (char*)"gps-lat";
        device->reference = (char*)"lat";
        return true;
        break;
   case 6:
        device->name = (char*)"gps-lon";
        device->reference = (char*)"lon";
        return true;
        break;
   case 7:
        device->name = (char*)"gps-sat";
        device->reference = (char*)"sat";
        return true;
        break;
   case 8:
        device->name = (char*)"accel-moved";
        device->reference = (char*)"moved";
        return true;
        break;
   case 9:
        device->name = (char*)"cell";
        device->reference = (char*)"rssi";
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
   /*running_task = true;
   wasm_thread = run_wasm_module(wasm_binary, wasm_binary_length, environment_keys, environment_int_values, environment_str_values, num_environment_variables);

   return true;*/
}

task_state_t agent_port_get_wasm_task_state(char** error_message) {

   /*uint32_t state =  wasm_thread->base.thread_state;

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
   }*/
}

bool agent_port_kill_wasm_task(void) {
   return true;
}
