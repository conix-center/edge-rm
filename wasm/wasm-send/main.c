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

    char filt[5];
    waGetEnvironmentString("FILT_FUNC", filt, 5);

    int fval;
    waGetEnvironmentInt("FILT_VAL", &fval, 1);

    int m = waGetMs();
    char sResult[20];
    int l;
    float f;
    while(waGetMs() - m < 120000) {
        waDelayMs(period*1000);
        f = waReadSensor(sensor, sResult, 20);
        if(filt[0] == 'G') {
            if(f > fval) {
                waCoapPost(ip, port, path, sResult, 20);
            }
        } else if (filt[0] == 'L') {
            if(f < fval) {
                waCoapPost(ip, port, path, sResult, 20);
            }
        } else {
            waCoapPost(ip, port, path, sResult, 20);
        }
    }

    return 0;
}
