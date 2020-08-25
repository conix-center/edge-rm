#!/usr/bin/env python3
import getopt
import socket
import sys
import psutil
import time
import argparse
import uuid
sys.path.insert(1, '../CoAPthon3')

from coapthon.client.helperclient import HelperClient
from coapthon import defines

import messages_pb2

client = None
framework_name = "Test Framework Name"
framework_id = "TEST ID"

def submitDummyTask(offers):
    print("Searching for a good offer...")
    slave_to_use = None
    resources_to_use = {}
    for i in reversed(range(len(offers))):
        offer = offers[i]
        if offer.slave_id:
            resources_to_use = {}
            for resource in offer.resources:
                if resource.name == "cpus" and resource.scalar.value >= 1:
                    resources_to_use["cpus"] = 1
                if resource.name == "mem" and resource.scalar.value > 100000000:
                    resources_to_use["mem"] = 100000000
            if len(resources_to_use) == 2:
                slave_to_use = offer.slave_id
            break
    if not slave_to_use:
        print("No available agents...")
        return

    print("Submitting task to agent " + slave_to_use + "...")

    # construct message
    wrapper = messages_pb2.WrapperMessage()
    wrapper.run_task.framework.name = framework_name
    wrapper.run_task.framework.framework_id = framework_id
    wrapper.run_task.task.name = "test task"
    wrapper.run_task.task.task_id = str(uuid.uuid1())
    wrapper.run_task.task.slave_id = slave_to_use
    for resource in resources_to_use:
        r = wrapper.run_task.task.resources.add()
        r.name = resource
        r.type = messages_pb2.Value.SCALAR
        r.scalar.value = resources_to_use[resource]
    # wrapper.run_task.task.resources.extend(resources_to_use)
    wrapper.run_task.task.container.type = messages_pb2.ContainerInfo.Type.DOCKER
    wrapper.run_task.task.container.docker.image = "hello-world"
    wrapper.run_task.task.container.docker.network = messages_pb2.ContainerInfo.DockerInfo.Network.HOST
    port_mapping = wrapper.run_task.task.container.docker.port_mappings.add()
    port_mapping.host_port = 3000
    port_mapping.container_port = 3000
    runtask_payload = wrapper.SerializeToString()
    ct = {'content_type': defines.Content_types["application/octet-stream"]}
    response = client.post('task', runtask_payload, timeout=2, **ct)
    if response:
        wrapper = messages_pb2.WrapperMessage()
        wrapper.ParseFromString(response.payload)
        print("Task Running!")
        #TODO: Generate confirmation protobuf message
        
    else:
        print("Failed to submit task...")
        client.stop()
        sys.exit(1)

def getOffer():
    # get offers
    print("Requesting resource offers...")
    wrapper = messages_pb2.WrapperMessage()
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


def main(host, port):  # pragma: no cover
    global client

    try:
        tmp = socket.gethostbyname(host)
        host = tmp
    except socket.gaierror:
        pass
    
    client = HelperClient(server=(host, int(port)))
    
    # TODO: Should we register the framework first?

    offers = getOffer()
    submitDummyTask(offers)

    client.stop()

    # loop ping/pong
    # try:
    #     while True:
    #         time.sleep(5)
    #         wrapper = messages_pb2.WrapperMessage()
    #         wrapper.ping.slave.id = agent_id
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
    args = parser.parse_args()
    main(args.host, args.port)
