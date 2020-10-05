#ifndef WASM_RUNTIME_API
#define WASM_RUNTIME_API

#include "wasm_export.h"

void waMQTTSNConnect(wasm_exec_env_t exec_env, char* clientID, int keepAlive, char* gateAddr, int port);
void waMQTTSNDisconnect(wasm_exec_env_t exec_env);
void waMQTTSNStart(wasm_exec_env_t exec_env, int port);
void waMQTTSNStop(wasm_exec_env_t exec_env);
void waMQTTSNPub(wasm_exec_env_t exec_env, char *data, int qos, int ID);
void waMQTTSNSub(wasm_exec_env_t exec_env, int qos, char *topicName);
int waMQTTSNReg(wasm_exec_env_t exec_env, char *topicName);
int waGetCPUCycles(wasm_exec_env_t exec_env);
int waConvertCyclesToMilis(wasm_exec_env_t exec_env ,int cycles);
float waRead(wasm_exec_env_t exec_env, char* sensorName, char* attr);
void waOpen(wasm_exec_env_t exec_env, char* sensorName);
void printConsole(wasm_exec_env_t exec_env, int time);

#endif
