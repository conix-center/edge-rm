#include "wasm_runtime_api.h"

#include <zephyr.h>

#include <net/openthread.h>
#include <openthread/cli.h>
#include <openthread/ip6.h>
#include <openthread/link.h>
#include <openthread/link_raw.h>
#include <openthread/ncp.h>
#include <openthread/message.h>
#include <openthread/platform/diag.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>
#include <openthread/dataset.h>
#include <openthread/joiner.h>
#include <openthread-system.h>
#include <openthread-config-generic.h>
#include "openthread/mqttsn.h"

#include "wasm_export.h"

#define GATEWAY_PORT 47193
#define GATEWAY_ADDRESS "fdde:ad00:beef:0:9a2a:2a02:71bf:f061"

#define CLIENT_ID "NRF52840"
#define CLIENT_PORT 10000

void mqttsnConnect(wasm_exec_env_t exec_env)
{
    otInstance *instance=openthread_get_default_instance();
    printf("Entered\n");
    otIp6Address address;
    otIp6AddressFromString(GATEWAY_ADDRESS, &address);
    otMqttsnConfig config;
    config.mClientId = CLIENT_ID;
    config.mKeepAlive = 30;
    config.mCleanSession = true;
    config.mPort = GATEWAY_PORT;
    config.mAddress = &address;
    config.mRetransmissionCount = 3;
    config.mRetransmissionTimeout = 10;
    otMqttsnConnect(instance, &config);
    //otMqttsnDisconnect(instance);
}

int get_time(){
    return k_cycle_get_32();
}

int convert(int cycles) {
    return SYS_CLOCK_HW_CYCLES_TO_NS(cycles);
}


