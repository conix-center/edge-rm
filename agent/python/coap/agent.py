#!/usr/bin/env python
import getopt
import socket
import sys
import psutil
import time
sys.path.insert(1, '../../../CoAPthon')

from coapthon.client.helperclient import HelperClient

import messages_pb2

client = None


def main():  # pragma: no cover
    global client
    
    # host = "128.97.92.77"
    # port = 3000
    host = "127.0.0.1"
    port = 3000

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
    mem_resource = wrapper.register_slave.slave.resources.add()
    mem_resource.name = "mem"
    mem_resource.type = messages_pb2.Value.SCALAR
    mem_resource.scalar.value = psutil.virtual_memory().available
    print("Memory Available:")
    print(mem_resource)

    register_payload = wrapper.SerializeToString()

    print("Registering with master...")

    # register with master
    response = client.post('register', register_payload, timeout=2)
    if response:
        wrapper = messages_pb2.WrapperMessage()
        wrapper.ParseFromString(response.payload)
        print("My Slave ID is " + wrapper.slave_registered.slave_id.value)
    else:
        print("Something went wrong...")
        client.stop()
        sys.exit(1)

    # loop ping/pong
    try:
        while True:
            time.sleep(5)
            wrapper = messages_pb2.WrapperMessage()
            wrapper.ping.connected = True
            response = client.get('ping', wrapper.SerializeToString(), timeout=2)
            if response:
                print("Pong!")
    except KeyboardInterrupt:
        print("Client Shutdown")
        # TODO: Deregister
        client.stop()


if __name__ == '__main__':  # pragma: no cover
    main()
