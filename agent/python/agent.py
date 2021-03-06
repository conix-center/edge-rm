#!/usr/bin/env python3
import getopt
import socket
import os
import sys
import sysconfig
import humanfriendly
import platform
import psutil
import time
import uuid
import argparse
import yaml
import dockerhelper
import wasmhelper
import socket
import db
import requests
import distro

import messages_pb2

agent_id = str(uuid.getnode())
agent_name = socket.gethostname()
ping_rate = 1000 #ping every 1000ms

def parseConfig(configPath):
    with open(configPath) as file:
        config = yaml.load(file, Loader=yaml.FullLoader)
        return config

def constructResources(resources, config):
    # add CPU
    cpu_resource = resources.add()
    cpu_resource.name = "cpus"
    cpu_resource.type = messages_pb2.Value.SCALAR
    cpu_list = psutil.cpu_percent(interval=1,percpu=True)
    cpu_value = 0
    for cpu in cpu_list:
        cpu_value += (100 - cpu)/100

    if 'maxCPUs' in config:
        if config['maxCPUs'] < cpu_value:
            cpu_value = config['maxCPUs']

    cpu_resource.scalar.value = cpu_value

    # add MEMORY
    mem_resource = resources.add()
    mem_resource.name = "mem"
    mem_resource.type = messages_pb2.Value.SCALAR
    mem = psutil.virtual_memory().available
    if 'maxMem' in config:
        maxmem = humanfriendly.parse_size(config['maxMem'])
        if maxmem < mem:
            mem = maxmem
    mem_resource.scalar.value = mem

    # add DISK
    disk_resource = resources.add()
    disk_resource.name = "disk"
    disk_resource.type = messages_pb2.Value.SCALAR
    disk = os.statvfs('./').f_frsize * os.statvfs('./').f_bavail
    if 'maxDisk' in config:
        maxdisk = humanfriendly.parse_size(config['maxDisk'])
        if maxdisk < disk:
            disk = maxdisk
    disk_resource.scalar.value = disk

    # add devices
    if config and 'devices' in config:
        for device in config['devices']:
            device_resource = resources.add()
            device_resource.name = device['name']
            device_resource.type = messages_pb2.Value.DEVICE
            device_resource.text.value =  device['id']
            if 'shared' in device:
                device_resource.shared = device['shared']
            else:
                device_resource.shared = False

def constructAttributes(attributes, config):
    #now add attributes
    #first the OS attribute
    os_attribute = attributes.add()
    os_attribute.name = "OS"
    os_attribute.type = messages_pb2.Value.TEXT
    #first see if we are on mac or linux then assign os in form
    #osname-version-arch
    os = None
    if platform.system().lower() == 'darwin':
        os = sysconfig.get_platform() 
    elif platform.system().lower() == 'linux':
        dist = distro.linux_distribution(full_distribution_name=False)[0]
        version = distro.linux_distribution(full_distribution_name=False)[1]
        arch = platform.machine()
        os = dist + '-' + version + '-' + arch
    os_attribute.text.value =  os

    # add the supported executors
    executors = attributes.add()
    executors.name = "executors"
    executors.type = messages_pb2.Value.SET
    executors.set.item.append("DOCKER")
    if wasmhelper.enabled():
        executors.set.item.append("WASM")

    #if there is a domain attribute add it
    if 'domain' in config:
        domain_attribute = attributes.add()
        domain_attribute.name = "domain"
        domain_attribute.type = messages_pb2.Value.TEXT
        domain_attribute.text.value =  config['domain']

def updateTasks():
    # iterate through the containers and update the state
    for task_id, task in db.tasks().items():
        if task.container.type == messages_pb2.ContainerInfo.Type.DOCKER:
            task.state = dockerhelper.getContainerStatus(task_id)
            if task.state == messages_pb2.TaskInfo.ERRORED:
                task.error_message = dockerhelper.getContainerLogs(task_id)
        elif task.container.type == messages_pb2.ContainerInfo.Type.WASM:
            # get task state
            task.state = wasmhelper.getTaskStatus(task_id)
    db.save()

def constructPing(wrapper, config, verbose):
    wrapper.type = messages_pb2.WrapperMessage.Type.PING
    wrapper.ping.agent.ping_rate = ping_rate
    wrapper.ping.agent.id = agent_id
    wrapper.ping.agent.name = agent_name

    constructResources(wrapper.ping.agent.resources, config)

    #print all attributes
    constructAttributes(wrapper.ping.agent.attributes, config)

    # add the state of tasks to the ping
    updateTasks()
    wrapper.ping.tasks.extend(db.tasks().values())

    if verbose:
        print("Resources")
        print(wrapper.ping.agent.resources)
        print("Attributes")
        print(wrapper.ping.agent.attributes)
        print("Tasks")
        print(wrapper.ping.tasks)

def main(host, port, configPath, verbose):  # pragma: no cover
    global agent_name
    global ping_rate

    try:
        tmp = socket.gethostbyname(host)
        host = tmp
    except socket.gaierror:
        pass

    db.load()
    dockerhelper.load()

    #get the devices configuration
    config = dict()
    if configPath:
        config = parseConfig(configPath)
        print(config)

    print("My Agent ID is " + agent_id)

    if 'name' in config:
        agent_name = config['name']
    if 'pingRate' in config:
            ping_rate = config['pingRate']

    print("My Agent name is " + agent_name)

    wasmhelper.establishWasmRuntime(config)

    # loop ping/pong
    try:
        while True:
            wrapper = messages_pb2.WrapperMessage()
            constructPing(wrapper, config, verbose)
            print("")
            print("Ping!")
            try:
                response = requests.post("http://" + host + ":" + str(port) + '/ping', data=wrapper.SerializeToString(), timeout=2, headers={'Content-Type':'application/protobuf'})
                if response and response.content:
                    print("Pong!")
                    wrapper = messages_pb2.WrapperMessage()
                    wrapper.ParseFromString(response.content)
                    if wrapper.pong.kill_task.task_id:
                        print("Received kill task request!")
                        dockerhelper.killContainer(wrapper.pong.kill_task.task_id)
                        wasmhelper.killTask(wrapper.pong.kill_task.task_id)
                    if wrapper.pong.run_task.task.name:
                        if wrapper.pong.run_task.task.container.type == messages_pb2.ContainerInfo.Type.DOCKER:
                            print("Received Docker Task!!")

                            print("Storing task")
                            db.set_task(wrapper.pong.run_task.task.task_id, wrapper.pong.run_task.task)

                            print("Launching task")
                            #for now just grab the container info. Let ping check the state on the next run
                            containerInfo = dockerhelper.runImageFromRunTask(wrapper.pong.run_task, config.get('devices', []))
                        elif wrapper.pong.run_task.task.container.type == messages_pb2.ContainerInfo.Type.WASM:
                            print("Received WebAssembly Task!!")
                            if not wasmhelper.enabled():
                                print("Agent cannot run this type of task")
                            else:
                                print("Storing task")
                                db.set_task(wrapper.pong.run_task.task.task_id, wrapper.pong.run_task.task)

                                print("Launching task")
                                wasmhelper.runModuleFromRunTask(wrapper.pong.run_task)
                        else:
                            print("Agent cannot run this type of task")
            except requests.exceptions.ConnectionError:
                print("ERROR: Connection Error")
            except requests.exceptions.ReadTimeout:
                print("WARNING: Ping Response timeout")

            time.sleep(ping_rate / 1000)

    except KeyboardInterrupt:
        wasmhelper.stopWasmRuntime()
        print("Shutting down...")
        # TODO: Deregister

if __name__ == '__main__':  # pragma: no cover
    parser = argparse.ArgumentParser(description='Launch the Resource Manager Agent')
    parser.add_argument('--host', required=True, help='The Master IP to register with.')
    parser.add_argument('--port', required=False, default=80, help='The Master port to register on.')
    parser.add_argument('--config', required=True, help='The path of the configuration file.')
    parser.add_argument('--verbose', action='store_true', help='Enable verbose console logging.')
    args = parser.parse_args()
    main(args.host, args.port, args.config, args.verbose)
