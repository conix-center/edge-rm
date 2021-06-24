#include "../wasm_runtime_api.h"

int main(int argc, char *argv[]){

    char ip[20];
    waGetEnvironmentString("IP", ip, 20);

    int port;
    waGetEnvironmentInt("PORT", &port, 1);

    char path[20];
    waGetEnvironmentString("PATH", path, 20);

    char sensor[10];
    waGetEnvironmentString("SENSOR", sensor, 10);

    int period;
    waGetEnvironmentInt("PERIOD", &period, 1);

    int m = waGetMs();
    char sResult[20];
    int l;
    float f;
    while(waGetMs() - m < 120000) {
        f = waReadSensor(sensor, sResult, 20);
        waCoapPost(ip, port, path, sResult, 20);
        waDelayMs(period*1000);
    }

    return 0;
}
