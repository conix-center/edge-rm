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
//#include "openthread/mqttsn.h"
#include "coap_help.h"

#include <drivers/sensor.h>

#define BME280 DT_INST(0, bosch_bme280)

#if DT_NODE_HAS_STATUS(BME280, okay)
#define BME280_LABEL DT_LABEL(BME280)
#else
#error Your devicetree has no enabled nodes with compatible "bosch,bme280"
#define BME280_LABEL "<none>"
#endif

#include "wasm_export.h"

#define GATEWAY_PORT 47193
#define GATEWAY_ADDRESS "fdde:ad00:beef:0:9a2a:2a02:71bf:f061"

struct device *dev = NULL;
otInstance *instance = NULL;
bool connected=false;
bool registered=false;
bool published=false;
bool subscribed=false;
int topicID=0;
//otMqttsnTopic* topic;

extern char** wasm_environment_keys;
extern char** wasm_environment_str_values;
extern int32_t** wasm_environment_values;
extern uint8_t wasm_num_environment_vars;

/*void HandlePublished(otMqttsnReturnCode aCode, void* aContext)
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
   // printf("register topic\n");
    registered=true;
    topicID=aTopic->mData.mTopicId;
    topic=aTopic;
    const char* tmp=otMqttsnGetTopicName(aTopic);
    //printk("Name: %s\n",tmp);
}

void HandleConnected(otMqttsnReturnCode aCode, void* aContext)
{
    //printf("Gateway Connected\n");
    connected=true;

}

void HandleDisconnected(otMqttsnDisconnectType aType, void* aContext)
{
    OT_UNUSED_VARIABLE(aType);
    OT_UNUSED_VARIABLE(aContext);
    //printf("Gateway Disconnected\n");
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
    k_sleep(Z_TIMEOUT_MS(500));
}

int waMQTTSNReg(wasm_exec_env_t exec_env, char *topicName)
{
    //k_sleep(Z_TIMEOUT_MS(1000));
    otMqttsnClientState cliState=otMqttsnGetState(instance);
     switch (cliState)
    {
    case kStateDisconnected:
        //printk("State: Disconnected\n");
        break;
    case kStateActive:
        //printk("State: Active\n");
        otMqttsnRegister(instance, topicName, HandleRegistered, (void *)instance);
        break;
    case kStateAsleep:
        //printk("State: Asleep\n");
        break;
    case kStateAwake:
        //printk("State: Awake\n");
        break;
    case kStateLost:
        //printk("State: Lost\n");
        break;
    }
   k_sleep(Z_TIMEOUT_MS(50));
   return topicID; 
}

void waMQTTSNPub(wasm_exec_env_t exec_env, char *data, int qos, int ID)
{
   k_sleep(Z_TIMEOUT_MS(50));
   //const char* data = "{\"temperature\":24.0}";
   int32_t length = strlen(data);
   //topic = otMqttsnCreateTopicName(topicName);
   *topic=otMqttsnCreateTopicId((otMqttsnTopicId)ID);
   //printk("topicID: %i, ID: %i\n",topicID, ID);
   otMqttsnClientState cliState=otMqttsnGetState(instance);
     switch (cliState)
    {
    case kStateDisconnected:
        //printk("State: Disconnected\n");
        break;
    case kStateActive:
        //printk("State: Active\n");
         otMqttsnPublish(instance, (const uint8_t*)data, length, kQos1, false, topic,
            HandlePublished, NULL);
        break;
    case kStateAsleep:
        //printk("State: Asleep\n");
        break;
    case kStateAwake:
        //printk("State: Awake\n");
        break;
    case kStateLost:
        //printk("State: Lost\n");
        break;
    }
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
    k_sleep(Z_TIMEOUT_MS(500));
    otMqttsnClientState cliState=otMqttsnGetState(instance);
     switch (cliState)
    {
    case kStateDisconnected:
        //printk("State: Disconnected\n");
        break;
    case kStateActive:
        //printk("State: Active\n");
        otMqttsnDisconnect(instance);
        break;
    case kStateAsleep:
        //printk("State: Asleep\n");
        break;
    case kStateAwake:
        //printk("State: Awake\n");
        break;
    case kStateLost:
        //printk("State: Lost\n");
        break;
    }
}*/

int waCoapPost(wasm_exec_env_t exec_env, char* ipv4Address, int port, char* path, char* sendBuf, int sendBufLen) {
   printk("WASM sending to %s:%d/%s\n",ipv4Address,port,path);

   if(port == 0) {
      port = 5683;
   }

   return send_coap_request(ipv4Address, port, path, sendBuf, sendBufLen);

   //For now just drop the response
}

int waConvertCyclesToNs(wasm_exec_env_t exec_env, int cycles){
    return k_cyc_to_ns_floor32(cycles);
}
int waConvertCyclesToMs(wasm_exec_env_t exec_env, int cycles){
    return k_cyc_to_ms_floor32(cycles);
}
int waGetNs(wasm_exec_env_t exec_env) {
    return k_cyc_to_ns_floor32(k_cycle_get_32());
}
int waGetMs(wasm_exec_env_t exec_env) {
    return k_cyc_to_ms_floor32(k_cycle_get_32());
}

int waGetCycles(wasm_exec_env_t exec_env){
    return k_cycle_get_32();
}

void waDelayMs(wasm_exec_env_t exec_env, int ms) {
   k_sleep(Z_TIMEOUT_MS(ms));
}

void printString(wasm_exec_env_t exec_env, char* str)
{
 	printk("%s", str);
	k_sleep(Z_TIMEOUT_MS(50));
}
void printInt(wasm_exec_env_t exec_env, int i)
{
 	printk("%d", i);
	k_sleep(Z_TIMEOUT_MS(50));
}
void printFloat(wasm_exec_env_t exec_env, float f)
{
 	printk("%f", f);
	k_sleep(Z_TIMEOUT_MS(50));
}

float waReadSensor(wasm_exec_env_t exec_env, char* attr, char* result, int len)
{
      struct sensor_value temp, press, humidity;
      float t,p,h;
      
      //If the device isn't initialized, lazily initialize it
      if(dev == NULL) {
	 dev = (struct device*)device_get_binding("BME280");
	 if (dev == NULL) {
	    printk("No device \"%s\" found; did initialization fail?\n", BME280_LABEL);
	    return -1;
	 } else {
	    printk("Found device \"%s\"\n", BME280_LABEL);
	 }
      }

      // Sample the sensor
      sensor_sample_fetch(dev);

      if(strcmp(attr, "humidity") == 0 ){
	 sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &humidity);

	 h=humidity.val1+humidity.val2/1000000.0;
	 k_sleep(K_MSEC(50));
	 printk("Got humidity: %d\n",(int)h);
	 sprintf(result,"%d",(int)h);
	 return h;
      } else if(strcmp(attr, "press") == 0) {
	 sensor_channel_get(dev, SENSOR_CHAN_PRESS, &press);

	 p=press.val1+press.val2/1000000.0;
	 k_sleep(K_MSEC(50));
	 printk("Got pressure: %d\n",(int)p);
	 sprintf(result,"%d",(int)p);
	 return p;
     } else if(strcmp(attr, "temp") == 0) {
	 sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);

	 t=temp.val1+temp.val2/1000000.0;
	 k_sleep(K_MSEC(50));
	 printk("Got temperature: %d\n",(int)t);
	 sprintf(result,"%d",(int)t);
	 return t;
     } else {
	return -1.0;
     }
}

int waGetEnvironmentInt(wasm_exec_env_t exec_env, char* key, int* val, int len) {

   for(uint8_t i = 0; i < wasm_num_environment_vars; i++) {
      if(strcmp(key, wasm_environment_keys[i]) == 0) {
	 *val = wasm_environment_values[i];
	 return 1;
      }
   }

   return 0;
}

int waGetEnvironmentString(wasm_exec_env_t exec_env, char* key, char* str, int len) {

   for(uint8_t i = 0; i < wasm_num_environment_vars; i++) {
      if(strcmp(key, wasm_environment_keys[i]) == 0) {
	 int slen = strlen(wasm_environment_str_values[i]);
	 if(len < slen) {
	    memcpy(str, wasm_environment_str_values[i], len);
	 } else {
	    memcpy(str, wasm_environment_str_values[i], slen);
	 }
	 return 1;
      }
   }

   return 0;
}
