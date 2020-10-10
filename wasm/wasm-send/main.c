#include "../wasm_runtime_api.h"

int main(int argc, char *argv[]){

    char ip[20];
    waGetEnvironmentString("IP", ip, 20);

    int port;
    waGetEnvironmentInt("PORT", &port, 1);

    char path[10];
    waGetEnvironmentString("PATH", path, 10);

    char sensor[10];
    waGetEnvironmentString("SENSOR", sensor, 10);

    int period;
    waGetEnvironmentInt("PERIOD", &period, 1);

    char filt[5];
    waGetEnvironmentString("FILT_FUNC", filt, 5);

    int fval;
    waGetEnvironmentInt("FILT_VAL", &fval, 1);

    int m = waGetMs();
    while(waGetMs() - m < 180000) {
        waDelayMs(period*1000);
        float f = waReadSensor(sensor);
        if(filt[0] == 'G') {
            if(f > fval) {
                waCoapPost(ip, port, path, (char*)&f, 4);
            }
        } else if (filt[0] == 'L') {
            if(f < fval) {
                waCoapPost(ip, port, path, (char*)&f, 4);
            }
        } else {
            waCoapPost(ip, port, path, (char*)&f, 4);
        }
    }

    return 0;
}
