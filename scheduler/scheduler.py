#!/usr/bin/env python3
import getopt
import socket
import sys
import psutil
import time
import argparse
import uuid
import json
sys.path.insert(1, '../support/CoAPthon3')

from coapthon.client.helperclient import HelperClient
from coapthon import defines

import messages_pb2

client = None
framework_name = "Test Framework"
framework_id = "TEST ID"

tasks = []
tasksfile = 'tasks.json'

def loadTasks():
    global tasks
    global framework_id
    global framework_name
    try:
        with open(tasksfile, 'r') as file:
            data = json.load(file)
            if 'framework' in data and 'id' in data['framework']:
                framework_id = data['framework']['id']
            if 'framework' in data and 'name' in data['framework']:
                framework_name = data['framework']['name']
            if 'tasks' in data:
                tasks = data['tasks']
    except:
        pass
    if len(tasks) > 0:
        print("Kill your tasks first!!!")
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

def submitRunTask(name, agent_to_use, resources_to_use, dockerimg, port_mappings, env_variables):
    print("Submitting task to agent " + agent_to_use + "...")
    # construct message
    wrapper = messages_pb2.WrapperMessage()
    wrapper.type = messages_pb2.WrapperMessage.Type.RUN_TASK
    wrapper.run_task.task.framework.name = framework_name
    wrapper.run_task.task.framework.framework_id = framework_id
    wrapper.run_task.task.name = name
    task_id = str(uuid.uuid1())
    wrapper.run_task.task.task_id = task_id
    wrapper.run_task.task.agent_id = agent_to_use
    for resource in resources_to_use:
        r = wrapper.run_task.task.resources.add()
        r.name = resource
        if r.name == 'picam':
            r.type = messages_pb2.Value.DEVICE
        else:
            r.type = messages_pb2.Value.SCALAR
            r.scalar.value = resources_to_use[resource]
    # wrapper.run_task.task.resources.extend(resources_to_use)
    wrapper.run_task.task.container.type = messages_pb2.ContainerInfo.Type.DOCKER
    wrapper.run_task.task.container.docker.image = dockerimg
    wrapper.run_task.task.container.docker.network = messages_pb2.ContainerInfo.DockerInfo.Network.HOST
    for host_port, container_port in port_mappings.items():
        port_mapping = wrapper.run_task.task.container.docker.port_mappings.add()
        port_mapping.host_port = host_port
        port_mapping.container_port = container_port
    wrapper.run_task.task.container.docker.environment_variables.extend(env_variables)
    runtask_payload = wrapper.SerializeToString()
    ct = {'content_type': defines.Content_types["application/octet-stream"]}
    response = client.post('task', runtask_payload, timeout=2, **ct)
    if response:
        wrapper = messages_pb2.WrapperMessage()
        wrapper.ParseFromString(response.payload)
        print("Task Running!")
        tasks.append({
            'name': 'test task',
            'task_id': task_id,
            'agent_id': agent_to_use
        })
        dumpTasks()
        #TODO: Generate confirmation protobuf message
        
    else:
        print("Failed to submit task...")
        client.stop()
        sys.exit(1)

def getCamTask(offers):
    print("Searching for a good camera...")
    agent_to_use = None
    resources_to_use = {
        "cpus":0.5,
        "mem":100000000,
        "picam":True
    }
    for i in reversed(range(len(offers))):
        offer = offers[i]
        if offer.agent_id:
            good_cpu = False
            good_mem = False
            got_cam = False
            for resource in offer.resources:
                if resource.name == "cpus" and resource.scalar.value >= 0.5:
                    good_cpu = True
                if resource.name == "mem" and resource.scalar.value >= 100000000:
                    good_mem = True
                if resource.name == "picam":
                    got_cam = True
            for attribute in offer.attributes:
                if good_cpu and good_mem and got_cam and attribute.name == "OS" and attribute.text.value == "debian-10.1-armv6l":
                    return (offer.agent_id, resources_to_use)
    print("Failed to find a good camera...")
    client.stop()
    sys.exit(1)

def getServerTask(offers):
    print("Searching for a good server...")
    agent_to_use = None
    resources_to_use = {
        "cpus":1,
        "mem":500000000
    }
    for i in reversed(range(len(offers))):
        offer = offers[i]
        if offer.agent_id:
            good_cpu = False
            good_mem = False
            for resource in offer.resources:
                if resource.name == "cpus" and resource.scalar.value >= 0.5:
                    good_cpu = True
                if resource.name == "mem" and resource.scalar.value >= 100000000:
                    good_mem = True
            for attribute in offer.attributes:
                if good_cpu and good_mem and attribute.name == "domain":
                    return (offer.agent_id, resources_to_use, attribute.text.value)
    print("Failed to find a server...")
    client.stop()
    sys.exit(1)

def printOffer(offers):
    agent_to_use = None
    resources_to_use = {}
    for i in reversed(range(len(offers))):
        offer = offers[i]
        if offer.agent_id:
            print("\n\nAGENT " + str(offer.agent_id))
            resources_to_use = {}
            print("Resources:")
            for resource in offer.resources:
                print("\t",resource.name, resource.type, resource.scalar.value, resource.device.device)
                if resource.name == "cpus" and resource.scalar.value >= 1:
                    resources_to_use["cpus"] = 1
                if resource.name == "mem" and resource.scalar.value > 100000000:
                    resources_to_use["mem"] = 100000000
            print("Attributes:")
            for attribute in offer.attributes:
                print("\t",attribute.name, attribute.type, attribute.scalar.value, attribute.text.value)
            if len(resources_to_use) == 2:
                agent_to_use = offer.agent_id
    if not agent_to_use:
        print("No available agents...")
        return

def submitTwoTasks(offers):
    print("Searching for a good server offer...")
    printOffer(offers)

    (camera_agent, camera_resources) = getCamTask(offers)
    print("Got a camera!")
    (server_agent, server_resources, server_domain) = getServerTask(offers)
    print("Got a server!")

    # return
    # print("Submitting task to agent " + agent_to_use + "...")
    submitRunTask("webserver endpoint", server_agent, server_resources, "jnoor/hellocameraserver:v1", {3003:3003}, ['SERVER_PORT=3003'])
    submitRunTask("camera task", camera_agent, camera_resources, "jnoor/cameraalpine:v1", {}, ["SERVER_HOST=http://" + server_domain + ":3003/image"])
    

def getOffer():
    # get offers
    print("Requesting resource offers...")
    wrapper = messages_pb2.WrapperMessage()
    wrapper.type = messages_pb2.WrapperMessage.Type.RESOURCE_REQUEST
    wrapper.request.framework_id = framework_id
    request_payload = wrapper.SerializeToString()
    ct = {'content_type': defines.Content_types["application/octet-stream"]}
    response = client.post('request', request_payload, timeout=2, **ct)
    if response:
        wrapper = messages_pb2.WrapperMessage()
        wrapper.ParseFromString(response.payload)
        offers = wrapper.offermsg.offers
        print("Got offers!")
        return offers
    else:
        print("Couldn't receive resource offer... ?")
        client.stop()
        sys.exit(1)


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
    loadTasks()
    # TODO: Should we register the framework first?

    offers = getOffer()
    submitTwoTasks(offers)

    client.stop()

    # loop ping/pong
    # try:
    #     while True:
    #         time.sleep(5)
    #         wrapper = messages_pb2.WrapperMessage()
    #         wrapper.ping.agent.id = agent_id
    #         print("")
    #         print("Ping!")
    #         response = client.post('ping', wrapper.SerializeToString(), timeout=2)
    #         if response:
    #             print("Pong!")
    # except KeyboardInterrupt:
    #     print("Client Shutdown")
    #     # TODO: Deregister
    #     client.stop()


if __name__ == '__main__':  # pragma: no cover
    parser = argparse.ArgumentParser(description='Launch a CoAP Resource Manager Framework')
    parser.add_argument('--host', required=True, help='the Edge RM Master IP to register with.')
    parser.add_argument('--port', required=False, default=5683, help='the Edge RM Master port to register on.')
    parser.add_argument('--tasks', required=True, help='the file containing the scheduler tasks.')
    args = parser.parse_args()
    main(args.host, args.port, args.tasks)
