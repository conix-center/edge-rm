#!/usr/bin/env python3

import sys
import argparse
import pydig
import os
import subprocess
import shutil
import hashlib
import time
from map_compiler import build

from edgerm.framework import Framework

# call map compiler to get out a schedulable wasm tasks
def compile_map(map_file, period):
    build(map_file, period)
    return 'out.wasm'

# checks to see if a reduce server is running, and if it isn't schedules it
def schedule_reduce_server(framework, offers, dest_ip, dest_port, task_id):

    agent = framework.getAgentInfoForRunningTask("ReduceServer")
    if agent:
        print("Reduce server running.")
        domain = framework.getAgentProperty(agent,'domain')
        agent_ip = pydig.query(domain,'A')[0]
        return agent_ip, 3004

    # reduce task needs to be scheduled
    server_agents = framework.findAgents(offers, {'domain':None,'cpus':0.5,'mem':100000000})
    if len(server_agents) == 0:
        print("No available reduce agents.\n\nA reduce agent is required. Please ensure that the cluster has enough resources.\n\nExiting.")
        sys.exit(1)

    # issue task and return IP / Port
    framework.runTask("ReduceServer", server_agents[0], docker_image='jnoor/sensor-reduce:v1', docker_port_mappings={3004:3004},environment={'SERVER_PORT':'3004'})
    print("Started reduce task on agent: {}".format(server_agents[0].agent_id))
    domain = framework.getAgentProperty(server_agents[0].agent_id,'domain')
    agent_ip = pydig.query(domain,'A')[0]
    return agent_ip, 3004

# Takes the reduce code and posts it to the reduce server
def issue_reduce_code(reduce_file, reduce_server_ip, reduce_server_port, task_id, reissue_tasks):
    p = os.system(' '.join(['node', 'reduce/scheduler-reduce.js', reduce_file, task_id, reduce_server_ip, str(reduce_server_port)]))
    print("Issued reduce code.")
    return 'reduce/' + task_id + '/data'

def poll_reduce_server(output_file, reduce_server_ip, reduce_server_port, task_id):
    p = os.system(' '.join(['node', 'reduce/fetch-reduce.js', output_file, task_id, reduce_server_ip, str(reduce_server_port)]))

# Looks for available sensors on which to run the map task and schedules them
def schedule_map(framework, offers, wasm_file, sensor, sensor_filters, reduce_server_ip, reduce_server_port, task_path, task_id):
    #setup the environment
    env = {}
    env['PATH'] = task_path
    env['IP'] = reduce_server_ip
    env['PORT'] = reduce_server_port

    map_agents = framework.findAgents(offers, {'executors':'WASM','cpus':0.1, sensor:None})

    #if(len(map_agents) == 0):
    #    print("No map agents found.")
    #    retu

    for agent in map_agents:
        #for each agent get the sensor handle and set the environment variable
        sensor_device = framework.getResourceProperty(agent.resources, sensor)
        env['SENSOR'] = sensor_device

        __location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__)))
        f = open(os.path.join(__location__, wasm_file), 'rb')
        framework.runTask(task_id,agent,wasm_binary=f.read(),environment=env)
        print("Started new map task on agent: {}".format(agent.agent_id))
        print()

# Schedules a simple fileserver so that we can store and print user results
def schedule_result_server():
    return None, None

# Gets new results from the map reduce file server and prints them for the users
def fetch_and_print_results():
    pass

if __name__ == '__main__':  # pragma: no cover
    parser = argparse.ArgumentParser(
        description='Runs a sensor map-reduce job on an EdgeRM cluster.')

    parser.add_argument('--host', required=True, help='the Edge RM Master IP to register with.')
    parser.add_argument('--port', required=False, default=5683, help='the Edge RM Master port to register on.')

    parser.add_argument(
        '--sensor', required=True, help='The sensor to sample. Call edge-rm get nodes to see what sensors are available.')

    parser.add_argument('--period', required=True, help='Sample period of the sensor in seconds.')

    parser.add_argument('--sensor-filter', action='append', nargs='+', help='Key=Value pairs that must be in sensor metadata. Appened in a list and ORed together (i.e. --sensor-filter room=bedroom room=kitchen room=library runs on sensors with those metadata values set.) Not yet implemented')

    parser.add_argument(
        '--map', help='Optional map task. A c file that implements at single "map" function.')

    parser.add_argument(
        '--reduce', required=True, help='Reduce task. A javascript file that implements a "reduce" function.')

    parser.add_argument(
        '--id', help='Allows user to specify the name of this map reduce task. Defaults to the combined name of the map,reduce,sensor,and period fields.')

    parser.add_argument('--reissue-tasks', dest='reissue', action='store_true', help='Should already running tasks for the same ID be killed and reissued. Defaults to False.')

    args = parser.parse_args()

    #extract an ID for this task
    id = None
    if args.id:
        id = args.id
    else:
        id = str(args.map) + '-' + str(args.reduce) + '-' + args.sensor + args.period

    h = hashlib.sha256()
    h.update(id.encode('utf-8'))
    id = h.hexdigest()[0:20]

    #declare a framework
    framework = Framework("Map Reduce", args.host, args.port)

    # check if the reduce task is already running
    offers = framework.getOffers()


    #get the map function
    wasm_file = compile_map(args.map, args.period)

    #schedule the reduce/result parts of the code
    result_ip, result_port = schedule_result_server()
    ip, port = schedule_reduce_server(framework, offers, result_ip, result_port, id)
    path = issue_reduce_code(args.reduce, ip, port, id, args.reissue)

    #if reissue, kill all currently running map tasks
    if args.reissue:
        framework.killTasksThatMatch(id)

    #schedule the map parts of the code - maybe call in a loop for multiple arguments?
    schedule_map(framework, offers, wasm_file, args.sensor, args.sensor_filter, ip, port, path, id)

    # clear output log
    with open('reduce.log', 'w') as fp:
        pass
    number_of_lines = 0
    loop_cnt = 0
    print("Polling reducer.")
    while True:
        time.sleep(4)
        poll_reduce_server('reduce.log', ip, port, id)
        loop_cnt += 1
        if loop_cnt % 5 == 0:
            # reissue map tasks every X iterations
            offers = framework.getOffers()
            schedule_map(framework, offers, wasm_file, args.sensor, args.sensor_filter, ip, port, path, id)


    #call fetch and print results in loop
    #fetch_and_print_results(result_ip, result_port)


