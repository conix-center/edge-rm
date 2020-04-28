#include "string.h"
#include "stddef.h"

#include "edge_hal.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_sdh.h"
#include "nrf_soc.h"
#include "nrf_gpio.h"
#include "nrf_power.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "thread_coap.h"
#include "thread_dns.h"
#include <openthread/message.h>

#include "permamote.h"

#include "simple_thread.h"

uint8_t enables[7] = {
   MAX44009_EN,
   ISL29125_EN,
   MS5637_EN,
   SI7021_EN,
   PIR_EN,
   I2C_SDA,
   I2C_SCL
};

static const otIp6Address unspecified_ipv6 =
{
    .mFields =
    {
        .m8 = {0}
    }
};

callback global_callback = NULL;

// Returns 0 for success 1 for failure
// Generic method for initializing anything necessary for other functions on hardware
uint8_t edge_hal_initialize(void) {
    nrf_power_dcdcen_set(1);

    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();

    nrf_gpio_cfg_output(LED_1);
    nrf_gpio_cfg_output(LED_2);
    nrf_gpio_cfg_output(LED_3);
    nrf_gpio_pin_set(LED_1);
    nrf_gpio_pin_set(LED_2);
    nrf_gpio_pin_set(LED_3);
    for (int i = 0; i < 7; i++) {
      nrf_gpio_cfg_output(enables[i]);
      nrf_gpio_pin_set(enables[i]);
    }

    nrf_gpio_cfg_output(LI2D_CS);
    nrf_gpio_cfg_output(SPI_MISO);
    nrf_gpio_cfg_output(SPI_MOSI);
    nrf_gpio_pin_set(LI2D_CS);
    nrf_gpio_pin_set(SPI_MISO);
    nrf_gpio_pin_set(SPI_MOSI);

    thread_config_t thread_config = {
      .channel = 25,
      .panid = 0xFACE,
      .sed = true,
      .poll_period = 10000,
      .child_period = 60000,
      .autocommission = true,
    };
    thread_init(&thread_config);

    return 0;
}

void edge_hal_process(void) {
    thread_process();
}

void edge_hal_sleep(void) {
    if (NRF_LOG_PROCESS() == false)
    {
      thread_sleep();
    }
}

void gateway_response_handler (void* context, otMessage* message, const
                                 otMessageInfo* message_info, otError result) {
  if (result == OT_ERROR_NONE) {
    NRF_LOG_INFO("Sent Message Successfully!");
  } else {
    NRF_LOG_INFO("Failed to send message! 0x%x", result);
  }

  uint8_t code = otCoapMessageGetCode(message);
  uint8_t upper = code >> 5;
  uint8_t lower = code & 0x1F;

  if (upper == 4) {
    if (lower == 4) {
      NRF_LOG_INFO("404 Not Found!");
    }
  }

  global_callback(upper, lower);
}

// Sends COAP message to the requests URI, and calls the callback when it is acknowledged
uint8_t edge_hal_coap_send(uint8_t* ipv4_address, const char* path, uint8_t* payload, uint8_t payload_len, callback acknowledge_callback) {

  global_callback = acknowledge_callback;
    
  //convert to openthread ipv6 address
  char address[20];
  otIp6Address dest_addr;
  snprintf(address,20,"64:ff9b::%02X%02X:%02X%02X",ipv4_address[0],ipv4_address[1],ipv4_address[2],ipv4_address[3]);
  otError err = otIp6AddressFromString(address, &dest_addr);

  if(err != OT_ERROR_NONE) {
    return err;
  }

  if (otIp6IsAddressEqual(&dest_addr, &unspecified_ipv6)) {
    return OT_ERROR_ADDRESS_QUERY;
  }

  otInstance * thread_instance = thread_get_instance();
  otCoapType coap_type = OT_COAP_TYPE_CONFIRMABLE;

  otError error = thread_coap_send(thread_instance, OT_COAP_CODE_PUT, coap_type, &dest_addr, path, payload, payload_len, gateway_response_handler);

  // increment sequence number if successful
  if (error == OT_ERROR_NONE) {
      return 0;
  } else {
      return error;
  }
}

// Mallocs and fills in an array of resources that the hardware offers
// Array should be zero padded (ResourceType NONE)
Resources* edge_hal_create_resources(void) {
    return NULL;
}

// Updates the available resoruces
uint8_t edge_hal_update_resources(void) {
    return 0;
}
