#include "../wasm_runtime_api.h"

int main(int argc, char *argv[]){
    int z1, z2;
    int a1, a2;
    int b1, b2;
    int c1, c2;
    int d1, d2;
    z1=waGetCPUCycles();
    waMQTTSNStart(1000);
    z2=waGetCPUCycles();
    a1=waGetCPUCycles();
    waMQTTSNConnect("WASM-MAIN",5000,"fdde:ad00:beef:0:2452:a60c:8ad7:58a9",47193);
    a2=waGetCPUCycles();
    int id;
    b1=waGetCPUCycles();
    id=waMQTTSNReg("Sensors");
    b2=waGetCPUCycles();
    c1=waGetCPUCycles();
    waMQTTSNPub("temperatue: 70c\n",1,id);
    c2=waGetCPUCycles();
    d1=waGetCPUCycles();
    waMQTTSNDisconnect();    
    d2=waGetCPUCycles();
    printConsole(z2-z1);
    printConsole(a2-a1);
    printConsole(b2-b1);
    printConsole(c2-c1);
    printConsole(d2-d1);
    return 0;
}
