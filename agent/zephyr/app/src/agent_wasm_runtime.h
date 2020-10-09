#ifndef AGENT_WASM_RUNTIME
#define AGENT_WASM_RUNTIME

#include <zephyr.h>

k_tid_t run_wasm_module(uint8_t* buf, uint32_t len, char* e_keys[], int32_t e_values[], char* e_str_values[], uint8_t num_env_vars);
void    cleanup_wasm_module(void);
bool    check_wasm_errored(void);
char*   get_wasm_error_message();

#endif
