#include <zephyr.h>
#include <stdio.h>

#include "agent_wasm_runtime.h"
#include "wasm_runtime_api.h"
#include <stdio.h>
#include <string.h>
#include "wasm_export.h"

#define CONFIG_GLOBAL_HEAP_BUF_SIZE 80072
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


// Define an array of stacks to use
#define MAIN_THREAD_PRIORITY 5
K_THREAD_STACK_ARRAY_DEFINE(iwasm_main_thread_stacks, 10, CONFIG_MAIN_THREAD_STACK_SIZE);

// Define 10 thread objects
wasm_thread_t wasm_threads[10];

bool check_wasm_errored(wasm_thread_t* wasm_thread) {
    return wasm_thread->wasm_errored;
}

char* get_wasm_error_message(wasm_thread_t* wasm_thread) {
    return wasm_thread->wasm_exception;
}

static void* app_instance_main(wasm_module_inst_t module_inst, wasm_thread_t* wasm_thread)
{
    const char *exception;

    wasm_application_execute_main(module_inst, app_argc, app_argv);
    if ((exception = wasm_runtime_get_exception(module_inst))) {
        printk("%s\n", exception);
        wasm_thread->wasm_errored = true;
        snprintk(wasm_thread->wasm_exception, EXCEPTION_LEN, "%s", exception);
    }
    return NULL;
}

// Global Heap for the WASM runtime
static char global_heap_buf[CONFIG_GLOBAL_HEAP_BUF_SIZE] = { 0 };

void cleanup_wasm_module(wasm_thread_t* wasm_thread) {
    /* destroy the module instance */
    if(wasm_thread->wasm_module_inst != NULL) {
        wasm_runtime_deinstantiate(wasm_thread->wasm_module_inst);
    }

    /* unload the module */
    if(wasm_thread->wasm_module != NULL) {
        wasm_runtime_unload(wasm_thread->wasm_module);
    }

    wasm_thread->thread_used = false;

    /* destroy runtime environment */
    /*if(wasm_thread->runtime_init) {
        wasm_runtime_destroy();
    }*/
}

void iwasm_main(void *arg1, void *arg2, void *arg3)
{
    //Interpret the arguments passed to the thread
    uint8_t* wasm_file_buf = (uint8_t*)arg1;
    uint32_t wasm_file_size = *((uint32_t*)arg2);
    wasm_thread_t* wasm_thread = (wasm_thread_t*)arg3;

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
        EXPORT_WASM_API_WITH_SIG(waReadSensor,"($*~)f"),
        EXPORT_WASM_API_WITH_SIG(printString,"($)"),
        EXPORT_WASM_API_WITH_SIG(printInt,"(i)"),
        EXPORT_WASM_API_WITH_SIG(printFloat,"(f)"),
        EXPORT_WASM_API_WITH_SIG(waCoapPost,"($i$*~)i"),
        EXPORT_WASM_API_WITH_SIG(waGetEnvironmentInt,"($*~)i"),
        EXPORT_WASM_API_WITH_SIG(waGetEnvironmentString,"($*~)i")
    };

    memset(&init_args, 0, sizeof(RuntimeInitArgs));

    // TODO
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
        wasm_thread->wasm_errored = true;
        snprintf(wasm_thread->wasm_exception, EXCEPTION_LEN, "Failed to init runtime");
        return;
    }

    wasm_thread->runtime_init = true;

    /* load WASM module */
    if (!(wasm_thread->wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size,
                                          error_buf, sizeof(error_buf)))) {
        printk("%s\n", error_buf);
        wasm_thread->wasm_errored = true;
        snprintf(wasm_thread->wasm_exception, EXCEPTION_LEN, "%s", error_buf);
    }

    /* instantiate the module */
    if (!(wasm_thread->wasm_module_inst = wasm_runtime_instantiate(wasm_thread->wasm_module,
                                                      CONFIG_APP_STACK_SIZE,
                                                      CONFIG_APP_HEAP_SIZE,
                                                      error_buf,
                                                      sizeof(error_buf)))) {
        printk("%s\n", error_buf);
        wasm_thread->wasm_errored = true;
        snprintf(wasm_thread->wasm_exception, EXCEPTION_LEN, "%s", error_buf);
    }

    k_sleep(Z_TIMEOUT_MS(500));

    app_instance_main(wasm_thread->wasm_module_inst, wasm_thread);

    k_sleep(Z_TIMEOUT_MS(500));

    cleanup_wasm_module(wasm_thread);
}


wasm_thread_t* run_wasm_module(uint8_t* buf, uint32_t len, char* e_keys[], int32_t e_values[], char* e_str_values[], uint8_t num_env_vars) {

    // Pass in the buffer containing the WASM module and its length to the thread
    printk("Starting WASM thread\n");

    // Get the correct stack and thread objects
    int8_t thread_idx = -1;
    for(uint8_t i = 0; i < 10; i++) {
        if(wasm_threads[i].thread_used == 0) {
            thread_idx = i;
        }
    }

    if(thread_idx == -1) {
        printk("No more threads available");
        return 0;
    }

    wasm_threads[thread_idx].wasm_errored = false;
    memset(wasm_threads[thread_idx].wasm_exception, 0, EXCEPTION_LEN);

    //set up the environment
    wasm_threads[thread_idx].wasm_environment_keys = e_keys;
    wasm_threads[thread_idx].wasm_environment_values = e_values;
    wasm_threads[thread_idx].wasm_environment_str_values = e_str_values;
    wasm_threads[thread_idx].wasm_num_environment_vars = num_env_vars;
    wasm_threads[thread_idx].wasm_buf_len = len;

    //setup the stack pointer
    //wasm_threads[thread_idx].thread_stack_pt = &iwasm_main_thread_stacks[thread_idx];

    wasm_threads[thread_idx].thread_id = k_thread_create(&wasm_threads[thread_idx].thread, iwasm_main_thread_stacks[thread_idx],
                                                        CONFIG_MAIN_THREAD_STACK_SIZE,
                                                        iwasm_main, buf, &wasm_threads[thread_idx].wasm_buf_len, &wasm_threads[thread_idx],
                                                        MAIN_THREAD_PRIORITY, 0, K_NO_WAIT);
    return &wasm_threads[thread_idx];
}
