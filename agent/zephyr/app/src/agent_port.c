
#include <zephyr.h>
#include "agent_port.h"
#include "coap_help.h"

agent_port_timer_cb local_cb;

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

}

void agent_port_coap_send(const char* destination, uint8_t* payload, uint32_t len) {
   start_coap_client();

   send_coap_request(payload, len);
}

void agent_port_print(const char * str) {
   printk(str);
}
