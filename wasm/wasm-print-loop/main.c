#include "../wasm_runtime_api.h"

int main(int argc, char *argv[]){

    for(int i = 0; i < 10; i++) {
        waDelayMs(5000);
        printString("Hellow WASM\n");
    }

    return 0;
}
