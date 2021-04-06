import docker
import time
import psutil
import messages_pb2
import humanfriendly
import requests

print("Loading docker runtime...")
try:
    client = docker.from_env()
except:
    print("===\n===\n===\nTHIS USUALLY MEANS DOCKER IS NOT RUNNING\n===\n===\n===")
    raise

WasmRuntimeContainer = None
WasmRuntimeName = "LocalWasmRuntime"
WasmRuntimePORT = 3002

def loadWasmRuntime():
    global WasmRuntimeContainer
    try:
        WasmRuntimeContainer = client.containers.get(WasmRuntimeName)
    except Exception as e:
        print("TRIED TO LOAD")
        print(e)
        pass

def stopWasmRuntime():
    global WasmRuntimeContainer
    if WasmRuntimeContainer:
        WasmRuntimeContainer.reload()
        if WasmRuntimeContainer.status == "running":
            print("Stopping Wasm Runtime...")
            WasmRuntimeContainer.stop()

def establishWasmRuntime(config):
    global WasmRuntimeContainer
    # load if exists
    loadWasmRuntime()

    # stop if running
    stopWasmRuntime()
    
    # prune stopped + old containers
    print("Pruning Docker Containers...")
    client.containers.prune()

    if 'loadWasm' in config and config['loadWasm'] == True:
        print("Loading Wasm Runtime...")
        total_mem = psutil.virtual_memory().available if 'maxMem' not in config else humanfriendly.parse_size(config['maxMem'])

        WasmRuntimeContainer = client.containers.run('jnoor/wasm-linux:v1',
                                                        cpu_quota=10000,
                                                        cpu_period=100000,
                                                        mem_limit=int(0.1 * total_mem),
                                                        detach=True,
                                                        name=WasmRuntimeName,
                                                        network_mode="bridge",
                                                        ports={WasmRuntimePORT:WasmRuntimePORT},
                                                        environment={},
                                                        # volumes=volumes,
                                                        )

def runWasmTask(name, wasmbinary):
    requests.post('http://localhost:' + str(WasmRuntimePORT) + '/task?name=' + str(name), 
                    data=wasmbinary, 
                    headers={"Content-Type":"application/wasm"})

def receiveWasmMsg(name, message):
    requests.post('http://localhost:' + str(WasmRuntimePORT) + '/task/data?name=' + str(name), 
                    data=message, 
                    headers={"Content-Type":"text/plain"})