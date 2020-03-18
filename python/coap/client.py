#!/usr/bin/env python
import getopt
import socket
import sys
sys.path.insert(1, './CoAPthon')

from coapthon.client.helperclient import HelperClient

import messages_pb2

client = None


def main():  # pragma: no cover
    global client
    op = "GET"
    payload = "test"
    
    host = "128.97.92.77"
    port = 3000
    path = 'basic'

    try:
        tmp = socket.gethostbyname(host)
        host = tmp
    except socket.gaierror:
        pass
    
    client = HelperClient(server=(host, port))
    

    # construct message
    wrapper = messages_pb2.WrapperMessage()
    # add CPU
    cpu_resource = wrapper.register_slave.slave.resources.add()
    cpu_resource.name = "cpu"
    cpu_resource.type = messages_pb2.Value.Type.SCALAR
    cpu_resource.scalar.value = 1

    # add MEMORY
    mem_resource = wrapper.register_slave.slave.resources.add()
    mem_resource.name = "mem"
    mem_resource.type = messages_pb2.Value.Type.SCALAR
    mem_resource.scalar.value = 1024

    register_payload = wrapper.SerializeToString()

    print("Registering with master...")

    response = client.post('register', register_payload, timeout=2)
    print("Got a response!")
    if response:
        print(response.pretty_print())
    else:
        print("Something went wrong...")
    client.stop()
    sys.exit(1)


if __name__ == '__main__':  # pragma: no cover
    main()
