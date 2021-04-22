#include "wasm_runtime_api.h"
#include <stdio.h>

#ifdef MAP_FILE
#include MAP_FILE
#endif

int main(int argc, char *argv[]){

    char ip[20];
    waGetEnvironmentString("IP", ip, 20);

    int port;
    waGetEnvironmentInt("PORT", &port, 1);

    char path[20];
    waGetEnvironmentString("PATH", path, 20);

    char sensor[10] = SENSOR;
    int period = PERIOD;

    char sResult[20];
    int l;
    float f;
    while(1) {

        //read the sensor
        f = waReadSensor(sensor, sResult, 20);

        //call the map function
        #ifdef MAP_FILE
        f = map(f);
        #endif

        //turn it into JSON
        sprintf(sResult, "{\"value\":%.3f}", f);
        
        //post the result
        waCoapPost(ip, port, path, sResult, 20);

        waDelayMs(period*1000);
    }

    return 0;
}
