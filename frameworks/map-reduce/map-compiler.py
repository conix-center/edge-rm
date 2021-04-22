#!/usr/bin/env python3

import sys
import argparse
import pydig
import os
import subprocess


def build(map_func, sensor, period):

    if map_func:
        # there is a map func - include it in the clang call

        rel_path = os.path.relpath(map_func, './map-wrapper/')

        subprocess.run(["/opt/wasi-sdk/bin/clang",
                        "--target=wasm32",
                        "-O1",
                        "-z",
                        "stack-size=4096",
                        "-Wl,--initial-memory=65536",
                        "--sysroot=/Users/adkins/Research/edge-rm/agent/zephyr/app/wamr/wamr-sdk/app/libc-builtin-sysroot",
                        "-Wl,--allow-undefined-file=/Users/adkins/Research/edge-rm/agent/zephyr/app/wamr/wamr-sdk/app/libc-builtin-sysroot/share/defined-symbols.txt",
                        "-Wl,--no-threads,--strip-all,--no-entry",
                        "-nostdlib",
                        "-Wl,--export=main",
                        "-Wl,--allow-undefined",
                        '-DMAP_FILE="' + rel_path + '"',
                        '-DSENSOR="' + sensor + '"',
                        "-DPERIOD=" + period,
                        "-o",
                        "out.wasm",
                        "./map-wrapper/main.c"])

    else:
        # there is no map func - import an empty map func
        subprocess.run(["/opt/wasi-sdk/bin/clang",
                        "--target=wasm32",
                        "-O1",
                        "-z",
                        "stack-size=4096",
                        "-Wl,--initial-memory=65536",
                        "--sysroot=/Users/adkins/Research/edge-rm/agent/zephyr/app/wamr/wamr-sdk/app/libc-builtin-sysroot",
                        "-Wl,--allow-undefined-file=/Users/adkins/Research/edge-rm/agent/zephyr/app/wamr/wamr-sdk/app/libc-builtin-sysroot/share/defined-symbols.txt",
                        "-Wl,--no-threads,--strip-all,--no-entry",
                        "-nostdlib",
                        "-Wl,--export=main",
                        "-Wl,--allow-undefined",
                        '-DSENSOR="' + sensor + '"',
                        "-DPERIOD=" + period,
                        "-o",
                        "out.wasm",
                        "./map-wrapper/main.c"])


if __name__ == '__main__':  # pragma: no cover
    parser = argparse.ArgumentParser(
        description='Builds a wasm file from a supplied c file that implements the map function')
    parser.add_argument(
        '--map', help='the Edge RM Master IP to register with.')
    parser.add_argument(
        '--sensor', required=True, help='The sensor. One of temp,press,humidity')
    parser.add_argument('--period', required=True, help='Sample period of the sensor')
    args = parser.parse_args()
    build(args.map, args.sensor, args.period)
