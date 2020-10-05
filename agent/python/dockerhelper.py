import docker
import time
import messages_pb2
client = docker.from_env()

containers = {}

def hello():
    print("hello")

def fetchImage(imageURL, forcepull=False):
    pullImage = False
    imageReady = False

    image = None
    try:
        image = docker.get(imageURL)
        imageReady = True
    except docker.errors.ImageNotFound:
        # need to pull image
        pullImage = True
    except docker.errors.APIError:
        print("Docker API Error. Exiting without executing task!")
        # should raise error
        return None

    if forcepull == True:
        pullImage = True

    if pullImage == True or imageReady == False:
        image = docker.pull(imageURL)
    
    if image == None:
        ## Maybe the image doesn't exist? Maybe docker.pull will throw and APIError?
        print("Image not found. Returning without executing task!")
        # should raise error
        return None

    return image

def runImage(image, cpu_shares, mem_limit, disk_limit, network, ports, environment, devices, volumes, frameworkName, taskID,):
    containerName = str(frameworkName + '-' + taskID).replace(" ","-")
    container = client.containers.run(image, 
                                      cpu_quota=int(cpu_shares), 
                                      cpu_period=100000, 
                                      mem_limit=int(mem_limit),
                                      storage_opt=disk_limit,
                                      detach=True, 
                                      name=containerName, 
                                      network_mode=network, 
                                      ports=ports,
                                      environment=environment,
                                      devices=devices,
                                      volumes=volumes)

    containers[taskID] = container

def killContainer(taskID):
    if taskID in containers:
        container = containers[taskID]
        print(container.status)
        if container.status == "running":
            container.kill()

def getContainerStatus(taskID):
    container = containers[taskID]
    container.reload()
    status = container.status
    print(status)
    if status == 'running':
        #This is good
        return messages_pb2.TaskInfo.RUNNING
    elif status == 'restarting' or status == 'created':
        #This is okay
        return messages_pb2.TaskInfo.STARTING
    elif status == 'exited' or status == 'dead' or status == 'removing':
        #Okay we need to get the status code to see what happened
        #this should exit immediately
        info = container.wait(timeout=1)
        if info['StatusCode'] == 0:
            return messages_pb2.TaskInfo.COMPLETED
        elif info['StatusCode'] == 137:
            return messages_pb2.TaskInfo.KILLED
        else:
            return messages_pb2.TaskInfo.ERRORED

def getContainerLogs(taskID):
    container = containers[taskID]
    return container.logs(tail=100)

def runImageFromRunTask(run_task, devices):
    imageName = run_task.task.container.docker.image
    frameworkName = run_task.task.framework.name
    taskID = run_task.task.task_id

    #setup cpu shared and memory limit
    cpu_shares = None
    mem_limit = None
    disk_limit = None
    offered_devices = list()
    for limit in run_task.task.resources:
        if limit.name == "cpus":
            cpu_shares = limit.scalar.value*100000
        if limit.name == "mem":
            mem_limit = limit.scalar.value
        if limit.name == "disk":
            disk_limit = {'size':limit.scalar.value}
        if limit.type == messages_pb2.Value.DEVICE:
            offered_devices.append(limit.name)

    #setup the networking
    network_setting = "host"
    if run_task.task.container.docker.network == messages_pb2.ContainerInfo.DockerInfo.Network.BRIDGE:
        network_setting = "bridge"
    if run_task.task.container.docker.network == messages_pb2.ContainerInfo.DockerInfo.Network.NONE:
        network_setting = "none"

    #map the ports
    ports = {}
    for port in run_task.task.container.docker.port_mappings:
        host = str(port.host_port)
        container = str(port.container_port)
        if port.protocol:
            container += "/" + port.protocol
        ports[container] = host

    #setup the environment variables from the run task message
    environment = list(run_task.task.container.docker.environment_variables)

    dockerDevices = list()
    dockerVolumes = {}
    for device in devices:
        if device['name'] not in offered_devices:
            continue
        
        if 'environmentVariables' in device:
            for env in device['environmentVariables']:
                environment.append(env)

        if 'dockerDevices' in device:
            for dev in device['dockerDevices']:
                dockerDevices.append(dev['hostPath'] + ':' + dev['containerPath'] + ':' + dev['permission'])

        if 'dockerVolumes' in device:
            for dev in device['dockerVolumes']:
                dockerVolumes[dev['hostPath']] = {'bind':dev['containerPath'], 'mode':dev['permission']}

    print(imageName, network_setting, ports, environment, dockerDevices, dockerVolumes)
    runImage(imageName, cpu_shares, mem_limit, disk_limit, network_setting, ports, environment, dockerDevices, dockerVolumes, frameworkName, taskID)
