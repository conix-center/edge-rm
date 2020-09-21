#include "wasm_runtime_api.h"

#include <zephyr.h>
#include <stdio.h>

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

otInstance *instance = NULL;
bool connected=false;
bool registered=false;
bool published=false;
bool subscribed=false;
int topicID=0;

void HandlePublished(otMqttsnReturnCode aCode, void* aContext)
{
    OT_UNUSED_VARIABLE(aCode);
    OT_UNUSED_VARIABLE(aContext);
    published=true;
}
void HandleSubscribed(otMqttsnReturnCode aCode, const otMqttsnTopic* aTopic, otMqttsnQos aQos, void* aContext)
{
    OT_UNUSED_VARIABLE(aCode);
    OT_UNUSED_VARIABLE(aTopic);
    OT_UNUSED_VARIABLE(aQos);
    OT_UNUSED_VARIABLE(aContext);
    subscribed=true;
}

void HandleRegistered(otMqttsnReturnCode aCode, const otMqttsnTopic* aTopic, void* aContext)
{
    printf("register topic\n");
    registered=true;
    topicID=aTopic->mData.mTopicId;
}

void HandleConnected(otMqttsnReturnCode aCode, void* aContext)
{
    printf("Gateway Connected\n");
    connected=true;

}

void HandleDisconnected(otMqttsnDisconnectType aType, void* aContext)
{
    OT_UNUSED_VARIABLE(aType);
    OT_UNUSED_VARIABLE(aContext);
    printf("Gateway Disconnected\n");
}

void waMQTTSNConnect(wasm_exec_env_t exec_env, char* clientID, int keepAlive, char* gatewayAddr, int gatewayPort)
{
    otIp6Address address;
    otIp6AddressFromString(gatewayAddr, &address);
    otMqttsnConfig config;
    config.mClientId = clientID;
    config.mKeepAlive = keepAlive;
    config.mCleanSession = true;
    config.mPort = gatewayPort;
    config.mAddress = &address;
    config.mRetransmissionCount = 3;
    config.mRetransmissionTimeout = 10;
    otMqttsnSetConnectedHandler(instance, HandleConnected, (void *)instance);
    otMqttsnSetDisconnectedHandler(instance, HandleDisconnected, (void *)instance);
    otMqttsnConnect(instance, &config); 
}

int waMQTTSNReg(wasm_exec_env_t exec_env, char *topicName)
{
   while(!connected){
    }
   otMqttsnRegister(instance, topicName, HandleRegistered, (void *)instance);
   while(!registered){
   }
   return topicID; 
}

void waMQTTSNPub(wasm_exec_env_t exec_env, char *data, int qos, char *topicName)
{
   while(!registered){
   }
   int32_t length = strlen(data);
   otMqttsnTopic topic = otMqttsnCreateTopicName(topicName);
   otMqttsnPublish(instance, (const uint8_t*)data, length, qos, false, &topic,
            HandlePublished, NULL);
}

void waMQTTSNSub(wasm_exec_env_t exec_env, int qos, char *topicName)
{
   while(!connected){
   }
   otMqttsnTopic topic = otMqttsnCreateTopicName(topicName);
   otMqttsnSubscribe(instance, &topic, qos, HandleSubscribed, NULL);
}

void waMQTTSNStart(wasm_exec_env_t exec_env, int port)
{
   instance=openthread_get_default_instance();
   otMqttsnStop(instance);
   otMqttsnStart(instance, port);
}

void waMQTTSNStop(wasm_exec_env_t exec_env)
{
   otMqttsnStop(instance);
}

void waMQTTSNDisconnect(wasm_exec_env_t exec_env)
{
    while(!connected){
    }
    otMqttsnDisconnect(instance);
}

int waGetCPUCycles(wasm_exec_env_t exec_env){
    return k_cycle_get_32();
}
int waConvertCyclesToMilis(wasm_exec_env_t exec_env, int cycles){
    return SYS_CLOCK_HW_CYCLES_TO_NS(cycles);
}
