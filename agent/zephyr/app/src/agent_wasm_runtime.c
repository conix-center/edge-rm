#include <zephyr.h>
#include <stdio.h>

#include "agent_wasm_runtime.h"
#include "wasm_runtime_api.h"
#include <stdio.h>
#include <string.h>
#include "wasm_export.h"

#define CONFIG_GLOBAL_HEAP_BUF_SIZE 131072
#define CONFIG_APP_STACK_SIZE 8192
#define CONFIG_APP_HEAP_SIZE 8192

#ifdef CONFIG_NO_OPTIMIZATIONS
#define CONFIG_MAIN_THREAD_STACK_SIZE 8192
#else
#define CONFIG_MAIN_THREAD_STACK_SIZE 4096
#endif

static int app_argc;
static char **app_argv;

/**
 * Find the unique main function from a WASM module instance
 * and execute that function.
 *
 * @param module_inst the WASM module instance
 * @param argc the number of arguments
 * @param argv the arguments array
 *
 * @return true if the main function is called, false otherwise.
 */
bool wasm_application_execute_main(wasm_module_inst_t module_inst,
                                    int argc, char *argv[]);

#define EXCEPTION_LEN 128
char  wasm_exception[EXCEPTION_LEN];
bool  wasm_errored = false;
uint32_t wasm_buf_len;

char** wasm_environment_keys;
int32_t* wasm_environment_values;
char** wasm_environment_str_values;
uint8_t wasm_num_environment_vars;

bool check_wasm_errored(void) {
    return wasm_errored;
}

char* get_wasm_error_message() {
    return wasm_exception;
}

static void* app_instance_main(wasm_module_inst_t module_inst)
{
    const char *exception;

    wasm_application_execute_main(module_inst, app_argc, app_argv);
    if ((exception = wasm_runtime_get_exception(module_inst))) {
        printk("%s\n", exception);
        wasm_errored = true;
        snprintk(wasm_exception, EXCEPTION_LEN, "%s", exception);
    }
    return NULL;
}

// Global Heap for the WASM runtime
static char global_heap_buf[CONFIG_GLOBAL_HEAP_BUF_SIZE] = { 0 };

wasm_module_t wasm_module = NULL;
wasm_module_inst_t wasm_module_inst = NULL;
bool runtime_init = false;

void cleanup_wasm_module(void) {
    /* destroy the module instance */
    if(wasm_module_inst != NULL) {
        wasm_runtime_deinstantiate(wasm_module_inst);
    }

    /* unload the module */
    if(wasm_module != NULL) {
        wasm_runtime_unload(wasm_module);
    }

    /* destroy runtime environment */
    if(runtime_init) {
        wasm_runtime_destroy();
    }
}

void iwasm_main(void *arg1, void *arg2, void *arg3)
{
    //Interpret the arguments passed to the thread
    uint8_t* wasm_file_buf = (uint8_t*)arg1;
    uint32_t wasm_file_size = *((uint32_t*)arg2);

    printk("Beginning of wasm thread main\n");
    printk("Got wasm binary with size %d\n", wasm_file_size);

    RuntimeInitArgs init_args;

    char error_buf[128];

    // Instantiate the symbols to pass into the WASM Module
    static NativeSymbol native_symbols[] =
    {
        EXPORT_WASM_API_WITH_SIG(waGetCycles,"()i"),
        EXPORT_WASM_API_WITH_SIG(waGetMs,"()i"),
        EXPORT_WASM_API_WITH_SIG(waGetNs,"()i"),
        EXPORT_WASM_API_WITH_SIG(waConvertCyclesToMs,"(i)i"),
        EXPORT_WASM_API_WITH_SIG(waConvertCyclesToNs,"(i)i"),
        EXPORT_WASM_API_WITH_SIG(waDelayMs,"(i)"),
        //EXPORT_WASM_API_WITH_SIG(waMQTTSNStart,"(i)"),
        //EXPORT_WASM_API_WITH_SIG(waMQTTSNStop,"()"),
        //EXPORT_WASM_API_WITH_SIG(waMQTTSNConnect,"($i$i)"),
        //EXPORT_WASM_API_WITH_SIG(waMQTTSNDisconnect,"()"),
        //EXPORT_WASM_API_WITH_SIG(waMQTTSNReg,"($)i"),
        //EXPORT_WASM_API_WITH_SIG(waMQTTSNPub,"($ii)"),
        //EXPORT_WASM_API_WITH_SIG(waMQTTSNSub,"(i$)"),
        EXPORT_WASM_API_WITH_SIG(waReadSensor,"($*~)f"),
        EXPORT_WASM_API_WITH_SIG(printString,"($)"),
        EXPORT_WASM_API_WITH_SIG(printInt,"(i)"),
        EXPORT_WASM_API_WITH_SIG(printFloat,"(f)"),
        EXPORT_WASM_API_WITH_SIG(waCoapPost,"($i$*~)i"),
        EXPORT_WASM_API_WITH_SIG(waGetEnvironmentInt,"($*~)i"),
        EXPORT_WASM_API_WITH_SIG(waGetEnvironmentString,"($*~)i")
    };

    memset(&init_args, 0, sizeof(RuntimeInitArgs));

    init_args.mem_alloc_type = Alloc_With_Pool;
    init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
    init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);

    // Native symbols need below registration phase
    init_args.n_native_symbols = sizeof(native_symbols) / sizeof(NativeSymbol);
    init_args.native_module_name = "env";
    init_args.native_symbols = native_symbols;

    /* initialize runtime environment */
    if (!wasm_runtime_full_init(&init_args)) {
        printk("Init runtime environment failed.\n");
        wasm_errored = true;
        snprintf(wasm_exception, EXCEPTION_LEN, "Failed to init runtime");
        return;
    }

    runtime_init = true;

    /* load WASM module */
    if (!(wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size,
                                          error_buf, sizeof(error_buf)))) {
        printk("%s\n", error_buf);
        wasm_errored = true;
        snprintf(wasm_exception, EXCEPTION_LEN, "%s", error_buf);
    }

    /* instantiate the module */
    if (!(wasm_module_inst = wasm_runtime_instantiate(wasm_module,
                                                      CONFIG_APP_STACK_SIZE,
                                                      CONFIG_APP_HEAP_SIZE,
                                                      error_buf,
                                                      sizeof(error_buf)))) {
        printk("%s\n", error_buf);
        wasm_errored = true;
        snprintf(wasm_exception, EXCEPTION_LEN, "%s", error_buf);
    }

    k_sleep(Z_TIMEOUT_MS(500));

    app_instance_main(wasm_module_inst);

    k_sleep(Z_TIMEOUT_MS(500));

    cleanup_wasm_module();
}



// Instantiate the thread and thread stack

#define MAIN_THREAD_STACK_SIZE (CONFIG_MAIN_THREAD_STACK_SIZE)
#define MAIN_THREAD_PRIORITY 5
K_THREAD_STACK_DEFINE(iwasm_main_thread_stack, MAIN_THREAD_STACK_SIZE);
static struct k_thread iwasm_main_thread;

k_tid_t run_wasm_module(uint8_t* buf, uint32_t len, char* e_keys[], int32_t e_values[], char* e_str_values[], uint8_t num_env_vars) {

    // Pass in the buffer containing the WASM module and its length to the thread
    printk("Starting WASM thread\n");

    //reset exception flags
    wasm_errored = false;
    memset(wasm_exception,0,EXCEPTION_LEN);

    //set up the environment
    wasm_environment_keys = e_keys;
    wasm_environment_values = e_values;
    wasm_environment_str_values = e_str_values;
    wasm_num_environment_vars = num_env_vars;

    wasm_buf_len = len;

    k_tid_t tid = k_thread_create(&iwasm_main_thread, iwasm_main_thread_stack,
                                  MAIN_THREAD_STACK_SIZE,
                                  iwasm_main, buf, &wasm_buf_len, NULL,
                                  MAIN_THREAD_PRIORITY, 0, K_NO_WAIT);
    return tid;
}
