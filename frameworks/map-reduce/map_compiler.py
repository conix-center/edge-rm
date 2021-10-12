#!/usr/bin/env python3

import sys
import argparse
import pydig
import os
import subprocess
import shutil


def build(map_func, period):

    #create a master undefined symbols file
    shutil.copy('../../agent/zephyr/app/wamr/wamr-sdk/app/libc-builtin-sysroot/share/defined-symbols.txt','../map-reduce/map-wrapper/defined-symbols.txt')

    f = open('../map-reduce/map-wrapper/defined-symbols.txt', 'a')
    a = open('../map-reduce/map-wrapper/wasm_runtime_api.txt')
    f.write(a.read())
    f.close()

    if map_func:
        # there is a map func - include it in the clang call

        rel_path = os.path.relpath(map_func, '../map-reduce/map-wrapper/')

        
        subprocess.check_output(["/usr/local/opt/llvm@9/bin/clang",
                        "--target=wasm32",
                        "-O1",
                        "-z",
                        "stack-size=4096",
                        "-Wl,--initial-memory=65536",
                        "--sysroot=../../agent/zephyr/app/wamr/wamr-sdk/app/libc-builtin-sysroot",
                        "-Wl,--allow-undefined-file=../map-reduce/map-wrapper/defined-symbols.txt",
                        "-Wl,--no-threads,--strip-all,--no-entry",
                        "-nostdlib",
                        "-Wl,--export=__heap_base",
                        "-Wl,--export=__data_end", 
                        "-Wl,--export=main",
                        '-DMAP_FILE="' + rel_path + '"',
                        "-DPERIOD=" + period,
                        "-o",
                        "out.wasm",
                        "../map-reduce/map-wrapper/main.c"])
        

    else:
        # there is no map func - import an empty map func
        subprocess.check_output(["/usr/local/opt/llvm@9/bin/clang",
                        "--target=wasm32",
                        "-O1",
                        "-z",
                        "stack-size=4096",
                        "-Wl,--initial-memory=65536",
                        "--sysroot=../../agent/zephyr/app/wamr/wamr-sdk/app/libc-builtin-sysroot",
                        "-Wl,--allow-undefined-file=../map-reduce/map-wrapper/defined-symbols.txt",
                        "-Wl,--no-threads,--strip-all,--no-entry",
                        "-nostdlib",
                        "-Wl,--export=__heap_base",
                        "-Wl,--export=__data_end", 
                        "-Wl,--export=main",
                        "-DPERIOD=" + period,
                        "-o",
                        "out.wasm",
                        "../map-reduce/map-wrapper/main.c"])


if __name__ == '__main__':  # pragma: no cover
    parser = argparse.ArgumentParser(
        description='Builds a wasm file from a supplied c file that implements the map function')
    parser.add_argument(
        '--map', help='the Edge RM Master IP to register with.')
    parser.add_argument('--period', required=True, help='Sample period of the sensor')
    args = parser.parse_args()
    build(args.map, args.period)
