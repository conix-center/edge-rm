#!/usr/bin/env python3
import getopt
import socket
import os
import sys
import psutil
import time
import argparse
import dockerhelper
coapPath = os.path.abspath("../../CoAPthon3")
sys.path.insert(1, coapPath)

from coapthon.client.helperclient import HelperClient
from coapthon import defines
#import docker as docker_client

import messages_pb2

client = None
agent_id = None
ping_rate = 1000 #ping every 1000ms

def addResourcesToPing(wrapper):
    wrapper.ping.slave.ping_rate = ping_rate

    # add CPU
    cpu_resource = wrapper.ping.slave.resources.add()
    cpu_resource.name = "cpus"
    cpu_resource.type = messages_pb2.Value.SCALAR
    cpu_list = psutil.cpu_percent(interval=1,percpu=True)
    cpu_value = 0
    for cpu in cpu_list:
        cpu_value += (100 - cpu)/100
    cpu_resource.scalar.value = cpu_value
    print("CPU Available:")
    print(cpu_resource)

    # add MEMORY
    mem_resource = wrapper.ping.slave.resources.add()
    mem_resource.name = "mem"
    mem_resource.type = messages_pb2.Value.SCALAR
    mem_resource.scalar.value = psutil.virtual_memory().available
    print("Memory Available:")
    print(mem_resource)

def main(host, port):  # pragma: no cover
    global client

    try:
        tmp = socket.gethostbyname(host)
        host = tmp
    except socket.gaierror:
        pass
    
    client = HelperClient(server=(host, int(port)))
    

    # construct message
    wrapper = messages_pb2.WrapperMessage()
    addResourcesToPing(wrapper)
    register_payload = wrapper.SerializeToString()

    print("Registering with master...")

    # register with master
    ct = {'content_type': defines.Content_types["application/octet-stream"]}
    response = client.post('ping', register_payload, timeout=2, **ct)
    if response:
        wrapper = messages_pb2.WrapperMessage()
        wrapper.ParseFromString(response.payload)
        agent_id = wrapper.pong.slave_id
        print("My Agent ID is " + wrapper.pong.slave_id)
    else:
        print("Something went wrong...")
        client.stop()
        sys.exit(1)

    # loop ping/pong
    try:
        while True:
            time.sleep(ping_rate / 1000)
            wrapper = messages_pb2.WrapperMessage()
            wrapper.ping.slave_id = agent_id
            addResourcesToPing(wrapper)
            print("")
            print("Ping!")
            ct = {'content_type': defines.Content_types["application/octet-stream"]}
            response = client.post('ping', wrapper.SerializeToString(), timeout=2, **ct)
            if response:
                print("Pong!")
                wrapper = messages_pb2.WrapperMessage()
                wrapper.ParseFromString(response.payload)
                if wrapper.run_task.task.name:
                    if wrapper.run_task.task.container.type == messages_pb2.ContainerInfo.Type.DOCKER:
                        print("Received Docker Task!!")
                        dockerhelper.runImageFromWrapper(wrapper)
    except KeyboardInterrupt:
        print("Client Shutdown")
        # TODO: Deregister
        client.stop()



if __name__ == '__main__':  # pragma: no cover
    parser = argparse.ArgumentParser(description='Launch the CoAP Resource Manager Agent')
    parser.add_argument('--host', required=True, help='the Master IP to register with.')
    parser.add_argument('--port', required=False, default=5683, help='the Master port to register on.')
    args = parser.parse_args()
    main(args.host, args.port)
    main()
