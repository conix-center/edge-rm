#include <zephyr.h>
#include <stdio.h>

#include "agent_wasm_runtime.h"
#include "wasm_runtime_api.h"

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

static void* app_instance_main(wasm_module_inst_t module_inst)
{
    const char *exception;

    wasm_application_execute_main(module_inst, app_argc, app_argv);
    if ((exception = wasm_runtime_get_exception(module_inst)))
        printf("%s\n", exception);
    return NULL;
}


// Global Heap for the WASM runtime
static char global_heap_buf[CONFIG_GLOBAL_HEAP_BUF_SIZE] = { 0 };

void iwasm_main(void *arg1, void *arg2, void *arg3)
{
    //Interpret the arguments passed to the thread
    uint8_t* wasm_file_buf = (uint8_t*)arg1;
    uint32_t wasm_file_size = *((uint32_t*)arg2);

    wasm_module_t wasm_module = NULL;
    wasm_module_inst_t wasm_module_inst = NULL;

    RuntimeInitArgs init_args;

    char error_buf[128];

    // Instantiate the symbols to pass into the WASM Module
    static NativeSymbol native_symbols[] =
    {

	{
            "get_time", 		// the name of WASM function name
            get_time, 			// the native function pointer
            "()i",			// the function prototype signature, avoid to use i32
            NULL                // attachment is NULL
        },
	{
            "convert", 		// the name of WASM function name
            convert, 			// the native function pointer
            "(i)i",			// the function prototype signature, avoid to use i32
            NULL                // attachment is NULL
        },
        {
            "mqttsnConnect", 		// the name of WASM function name
            mqttsnConnect, 			// the native function pointer
            "()",			// the function prototype signature, avoid to use i32
            NULL                // attachment is NULL
        }
	

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
        printf("Init runtime environment failed.\n");
        return;
    }


    /* load WASM module */
    if (!(wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size,
                                          error_buf, sizeof(error_buf)))) {
        printf("%s\n", error_buf);
    }

    /* instantiate the module */
    if (!(wasm_module_inst = wasm_runtime_instantiate(wasm_module,
                                                      CONFIG_APP_STACK_SIZE,
                                                      CONFIG_APP_HEAP_SIZE,
                                                      error_buf,
                                                      sizeof(error_buf)))) {
        printf("%s\n", error_buf);
    }

    /* invoke the main function */
    app_instance_main(wasm_module_inst);

    /* destroy the module instance */
    wasm_runtime_deinstantiate(wasm_module_inst);

    /* unload the module */
    wasm_runtime_unload(wasm_module);

    /* destroy runtime environment */
    wasm_runtime_destroy();
}

// Instantiate the thread and thread stack

#define MAIN_THREAD_STACK_SIZE (CONFIG_MAIN_THREAD_STACK_SIZE)
#define MAIN_THREAD_PRIORITY 5
K_THREAD_STACK_DEFINE(iwasm_main_thread_stack, MAIN_THREAD_STACK_SIZE);
static struct k_thread iwasm_main_thread;

k_tid_t run_wasm_module(uint8_t* buf, uint32_t len) {

    // Pass in the buffer containing the WASM module and its length to the thread
    k_tid_t tid = k_thread_create(&iwasm_main_thread, iwasm_main_thread_stack,
                                  MAIN_THREAD_STACK_SIZE,
                                  iwasm_main, buf, &len, NULL,
                                  MAIN_THREAD_PRIORITY, 0, K_NO_WAIT);
    return tid;
}