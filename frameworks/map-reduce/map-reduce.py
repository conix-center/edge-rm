#!/usr/bin/env python3

import sys
import argparse
import pydig
import os
import subprocess
import shutil
import hashlib

# call map compiler to get out a schedulable wasm tasks
def compile_map():
    pass

# checks to see if a reduce server is running, and if it isn't schedules it
def schedule_reduce_server():
    pass

# Takes the reduce code and posts it to the reduce server
def issue_reduce_code():
    pass

# Looks for available sensors on which to run the map task and schedules them
def schedule_map():
    pass

# Schedules a simple fileserver so that we can store and print user results
def schedule_result_server():
    pass

# Gets new results from the map reduce file server and prints them for the users
def fetch_and_print_results():
    pass

if __name__ == '__main__':  # pragma: no cover
    parser = argparse.ArgumentParser(
        description='Runs a sensor map-reduce job on an EdgeRM cluster.')

    parser.add_argument(
        '--sensor', required=True, help='The sensor to sample. Call edge-rm get nodes to see what sensors are available.')

    parser.add_argument('--period', required=True, help='Sample period of the sensor in seconds.')

    parser.add_argument('--sensor-filter', action='append', nargs='+', help='Key=Value pairs that must be in sensor metadata. Appened in a list and ORed together (i.e. --sensor-filter room=bedroom room=kitchen room=library runs on sensors with those metadata values set.) Not yet implemented')

    parser.add_argument(
        '--map', help='Optional map task. A c file that implements at single "map" function.')

    parser.add_argument(
        '--reduce', help='Optional reduce task. A javascript file that implements a "reduce" function.')

    parser.add_argument(
        '--id', help='Allows user to specify the name of this map reduce task. Defaults to the combined name of the map,reduce,sensor,and period fields.')

    parser.add_argument('--reissue-tasks', action=default=False, action=argparse.BooleanOptionalAction, help='Should already running tasks for the same ID be killed and reissued. Defaults to False.')

    args = parser.parse_args()

    #extract an ID for this task
    id = None
    if args.id:
        id = args.id
    else:
        id = [args.map,args.reduce,args.sensor,args.period]
        id = '-'.join(id)

    h = hashlib.sha256()
    h.update(id.encode('utf-8'))
    id = h.hexdigest()[0:20]


    #get the map function
    wasm_file = compile_map(args.map, args.sensor, args.period)


    #schedule the reduce/result parts of the code
    #result_ip, result_port = schedule_result_server(id)
    ip, port = schedule_reduce_server(result_ip, result_port, id)
    path = issue_reduce_code(args.reduce, id, ip, port, args.reissue_tasks)

    #schedule the map parts of the code - maybe call in a loop for multiple arguments?
    schedule_map(wasm_file, args.sensor_filter, id, ip, port, path, args.reissue_tasks)

    #call fetch and print results in loop
    #fetch_and_print_results(result_ip, result_port)


