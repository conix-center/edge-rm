#ifndef WASM_RUNTIME_API
#define WASM_RUNTIME_API

#include "wasm_export.h"

void mqttsnConnect(wasm_exec_env_t exec_env);
int get_time(void);
int convert(int cycles);

#endif
