#ifndef WASM_RUNTIME_API
#define WASM_RUNTIME_API

#include "wasm_export.h"

// MQTT SN Functions
void waMQTTSNConnect(wasm_exec_env_t exec_env, char* clientID, int keepAlive, char* gateAddr, int port);
void waMQTTSNDisconnect(wasm_exec_env_t exec_env);
void waMQTTSNStart(wasm_exec_env_t exec_env, int port);
void waMQTTSNStop(wasm_exec_env_t exec_env);
void waMQTTSNPub(wasm_exec_env_t exec_env, char *data, int qos, int ID);
void waMQTTSNSub(wasm_exec_env_t exec_env, int qos, char *topicName);
int waMQTTSNReg(wasm_exec_env_t exec_env, char *topicName);

// Sends coap data - Fills out rcvBuf - Returns bytes rcv
int waCoapPost(wasm_exec_env_t exec_env, char* ipv4Address, uint8_t* sendBuf, uint32_t sendBufLen, uint8_t* rcvBuf, uint32_t rcvBufLen, uint32_t timeout);

// Sensor read functions
float waReadSensor(wasm_exec_env_t exec_env, char* attr);

// Timing functions
int waGetCycles(wasm_exec_env_t exec_env);
int waGetNs(wasm_exec_env_t exec_env);
int waGetMs(wasm_exec_env_t exec_env);
int waConvertCyclesToNs(wasm_exec_env_t exec_env ,int cycles);
int waConvertCyclesToMs(wasm_exec_env_t exec_env ,int cycles);
void waDelayMs(wasm_exec_env_t exec_env, int ms);

// Printing functions
void printString(wasm_exec_env_t exec_env, char* str);
void printInt(wasm_exec_env_t exec_env, int i);
void printFloat(wasm_exec_env_t exec_env, float f);

// Environment functions
int waGetEnvironmentInt(wasm_exec_env_t exec_env, char* key, uint32_t* val, uint32_t len);
int waGetEnvironmentString(wasm_exec_env_t exec_env, char* key, char* str, uint32_t len);

#endif
