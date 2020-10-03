#!/usr/bin/env python3
import socket
import sys
import argparse
import json
sys.path.insert(1, '../CoAPthon3')

from coapthon.client.helperclient import HelperClient
from coapthon import defines

import messages_pb2

client = None
tasks = []
tasksfile = 'tasks.json'
framework_id = None
framework_name = None

def loadTasks():
    global tasks
    global framework_id
    global framework_name
    with open(tasksfile, 'r') as file:
        data = json.load(file)
        if 'framework' in data and 'id' in data['framework']:
            framework_id = data['framework']['id']
        if 'framework' in data and 'name' in data['framework']:
            framework_name = data['framework']['name']
        if 'tasks' in data:
            tasks = data['tasks']
        if len(tasks) == 0:
            print("No tasks to kill!")
            client.stop()
            sys.exit(1)

def dumpTasks():
    with open(tasksfile, 'w') as file:
        file.write(json.dumps({
            'framework': {
                'id': framework_id,
                'name': framework_name
            },
            'tasks': tasks
        }))

def killTasks():
    killed = []
    index = -1
    for task in tasks:
        index += 1
        print("Killing task...")
        wrapper = messages_pb2.WrapperMessage()
        wrapper.kill_task.name = task['name']
        wrapper.kill_task.task_id = task['task_id']
        wrapper.kill_task.agent_id = task['agent_id']
        wrapper.kill_task.framework.name = framework_name
        wrapper.kill_task.framework.framework_id = framework_id
        request_payload = wrapper.SerializeToString()
        ct = {'content_type': defines.Content_types["application/octet-stream"]}
        response = client.post('kill', request_payload, timeout=2, **ct)
        if response:
            wrapper = messages_pb2.WrapperMessage()
            wrapper.ParseFromString(response.payload)
            print("Kill task issued!")
            killed.insert(0,index)
        else:
            print("Couldn't issue kill task... ?")
    for idx in killed:
        del tasks[idx]
    dumpTasks()


def main(host, port, tasks):  # pragma: no cover
    global client
    global tasksfile

    tasksfile = tasks

    try:
        tmp = socket.gethostbyname(host)
        host = tmp
    except socket.gaierror:
        pass
    
    client = HelperClient(server=(host, int(port)))
    
    # TODO: Should we register the framework first?

    loadTasks()
    killTasks()

    client.stop()


if __name__ == '__main__':  # pragma: no cover
    parser = argparse.ArgumentParser(description='Launch a CoAP Resource Manager Framework')
    parser.add_argument('--host', required=True, help='the Edge RM Master IP to register with.')
    parser.add_argument('--port', required=False, default=5683, help='the Edge RM Master port to register on.')
    parser.add_argument('--tasks', required=True, help='the file containing the scheduler tasks')
    args = parser.parse_args()
    main(args.host, args.port, args.tasks)
