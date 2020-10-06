#ifndef WASM_RUNTIME_API
#define WASM_RUNTIME_API

void waMQTTSNConnect(char* clientID, int keepAlive, char* gateAddr, int port);
void waMQTTSNDisconnect();
void waMQTTSNStart(int port);
void waMQTTSNStop();
void waMQTTSNPub(char *data, int qos, int ID);
void waMQTTSNSub(int qos, char *topicName);
int waMQTTSNReg(char *topicName);
int waGetCPUCycles();
int waConvertCyclesToMilis(int cycles);
float waRead(char* sensorName, char* attr);
void waOpen(char* sensorName);
void printConsole(int time);

#endif
