#!/bin/bash

/opt/wasi-sdk/bin/clang --target=wasm32 -O0 -z stack-size=4096 -Wl,--initial-memory=65536 \
                        --sysroot=/Users/adkins/Research/edge-rm/agent/zephyr/app/wamr/wamr-sdk/app/libc-builtin-sysroot \
                        -Wl,--allow-undefined-file=/Users/adkins/Research/edge-rm/agent/zephyr/app/wamr/wamr-sdk/app/libc-builtin-sysroot/share/defined-symbols.txt \
                        -Wl,--no-threads,--strip-all,--no-entry -nostdlib \
                        -Wl,--export=main        -Wl,--allow-undefined \
                        -o out.wasm main.c
