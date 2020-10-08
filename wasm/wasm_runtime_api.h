#ifndef WASM_RUNTIME_API
#define WASM_RUNTIME_API


// MQTT SN Functions
void waMQTTSNConnect(char* clientID, int keepAlive, char* gateAddr, int port);
void waMQTTSNDisconnect(void);
void waMQTTSNStart(int port);
void waMQTTSNStop(void);
void waMQTTSNPub(char *data, int qos, int ID);
void waMQTTSNSub(int qos, char *topicName);
int waMQTTSNReg(char *topicName);

// Sends coap data - Fills out rcvBuf - Returns bytes rcv
int waCoapPost(char* ipv4Address, char* sendBuf, int sendBufLen, char* rcvBuf, int rcvBufLen, int timeout);

// Sensor read functions
float waReadSensor(char* attr);

// Timing functions
int waGetCycles(void);
int waGetNs(void);
int waGetMs(void);
int waConvertCyclesToNs(int cycles);
int waConvertCyclesToMs(int cycles);
void waDelayMs(int ms);

// Printing functions
void printString(char* str);
void printInt(int i);
void printFloat(float f);

// Environment functions
int waGetEnvironmentInt(char* key, int* val, int len);
int waGetEnvironmentString(char* key, char* str, int len);

#endif
