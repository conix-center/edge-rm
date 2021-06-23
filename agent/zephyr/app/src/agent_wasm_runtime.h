#ifndef AGENT_WASM_RUNTIME
#define AGENT_WASM_RUNTIME

#include "wasm_runtime_api.h"
#include "wasm_export.h"
#include <zephyr.h>

#define EXCEPTION_LEN 128
typedef struct _wasm_thread {
    k_thread_stack_t* thread_stack_pt;
    struct k_thread thread;
    k_tid_t thread_id;
    char wasm_exception[EXCEPTION_LEN];
    bool wasm_errored;
    uint32_t wasm_buf_len;
    char** wasm_environment_keys;
    int32_t* wasm_environment_values;
    char** wasm_environment_str_values;
    uint8_t wasm_num_environment_vars;
    wasm_module_t wasm_module;
    wasm_module_inst_t wasm_module_inst;
    bool runtime_init;
    bool thread_used;
} wasm_thread_t;

wasm_thread_t* run_wasm_module(uint8_t* buf, uint32_t len, char* e_keys[], int32_t e_values[], char* e_str_values[], uint8_t num_env_vars);
void    cleanup_wasm_module(wasm_thread_t* wasm_thread);
bool    check_wasm_errored(wasm_thread_t* wasm_thread);
char*   get_wasm_error_message(wasm_thread_t* wasm_thread);

#endif
