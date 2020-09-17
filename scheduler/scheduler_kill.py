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
framework_id = None
framework_name = None

def loadTasks(tasksfile):
    global tasks
    global framework_id
    global framework_name
    with open(tasksfile, 'r') as file:
        data = json.load(file)
        framework_id = data['framework']['id']
        framework_name = data['framework']['name']
        tasks = data['tasks']

def killTasks():
    for task in tasks:
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
        else:
            print("Couldn't issue kill task... ?")
            client.stop()
            sys.exit(1)


def main(host, port, tasks):  # pragma: no cover
    global client

    try:
        tmp = socket.gethostbyname(host)
        host = tmp
    except socket.gaierror:
        pass
    
    client = HelperClient(server=(host, int(port)))
    
    # TODO: Should we register the framework first?

    loadTasks(tasks)
    killTasks()

    client.stop()


if __name__ == '__main__':  # pragma: no cover
    parser = argparse.ArgumentParser(description='Launch a CoAP Resource Manager Framework')
    parser.add_argument('--host', required=True, help='the Edge RM Master IP to register with.')
    parser.add_argument('--port', required=False, default=5683, help='the Edge RM Master port to register on.')
    parser.add_argument('--tasks', required=True, help='the file containing the scheduler tasks')
    args = parser.parse_args()
    main(args.host, args.port, args.tasks)
