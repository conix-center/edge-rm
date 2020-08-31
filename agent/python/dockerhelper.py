import docker
import time
import messages_pb2
client = docker.from_env()

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

def runImage(image, cpu_shares, mem_limit, network, ports, frameworkName,taskID):
    containerName = str(frameworkName + '-' + taskID).replace(" ","-")
    container = client.containers.run(image,cpu_quota=int(cpu_shares),cpu_period=100000,mem_limit=int(mem_limit),network_mode=network,ports=ports,detach=True,name=containerName)
    # container = client.containers.run(image,detach=True)
    # print(container.logs())
    time.sleep(5)
    print(container.logs())

def runImageFromRunTask(run_task):
    imageName = run_task.task.container.docker.image
    frameworkName = run_task.task.framework.name
    taskID = run_task.task.task_id
    cpu_shares = None
    mem_limit = None
    network_setting = "host"
    if run_task.task.container.docker.network == messages_pb2.ContainerInfo.DockerInfo.Network.BRIDGE:
        network_setting = "bridge"
    if run_task.task.container.docker.network == messages_pb2.ContainerInfo.DockerInfo.Network.NONE:
        network_setting = "none"
    ports = {}
    for port in run_task.task.container.docker.port_mappings:
        host = str(port.host_port)
        container = str(port.container_port)
        if port.protocol:
            container += "/" + port.protocol
        ports[container] = host
    for limit in run_task.task.resources:
        if limit.name == "cpus":
            cpu_shares = limit.scalar.value*100000
        if limit.name == "mem":
            mem_limit = limit.scalar.value
    print(imageName, network_setting, ports)
    runImage(imageName, cpu_shares, mem_limit, network_setting, ports,frameworkName,taskID)
