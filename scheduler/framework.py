#!/usr/bin/env python
import getopt
import socket
import sys
import psutil
import time
import argparse
sys.path.insert(1, '../CoAPthon')

from coapthon.client.helperclient import HelperClient

import messages_pb2

client = None
framework_name = "Test Framework Name"
framework_id = "TEST ID"

def submitDummyTask(offers):
    # TODO: find an offer that works for you...

    print("Submitting task...")

    # construct message
    wrapper = messages_pb2.WrapperMessage()
    wrapper.run_task.framework.name = framework_name
    wrapper.run_task.framework.framework_id.value = framework_id
    wrapper.run_task.task.name = "test task"
    wrapper.run_task.task.name = "12D3"
    wrapper.run_task.task.task_id.value = "12D3"
    wrapper.run_task.task.slave_id.value = "1"
    wrapper.run_task.task.container.type = messages_pb2.ContainerInfo.Type.DOCKER
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
    wrapper.request.framework_id.value = framework_id
    request_payload = wrapper.SerializeToString()
    ct = {'content_type': defines.Content_types["application/octet-stream"]}
    response = client.post('request', request_payload, timeout=2, **ct)
    if response:
        wrapper = messages_pb2.WrapperMessage()
        wrapper.ParseFromString(response.payload)
        offers = wrapper.offer
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
    
    offers = getOffer()
    submitDummyTask(offers)

    client.stop()

    # loop ping/pong
    # try:
    #     while True:
    #         time.sleep(5)
    #         wrapper = messages_pb2.WrapperMessage()
    #         wrapper.ping.slave_id.value = agent_id
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
    parser.add_argument('--port', required=True, help='the Edge RM Master port to register on.')
    args = parser.parse_args()
    main(args.host, args.port)
