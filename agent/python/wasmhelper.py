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

def enabled():
    return WasmRuntimeContainer is not None

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
            WasmRuntimeContainer = None

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

def runWasmTask(name, env, wasmbinary):
    queryParams = env
    queryParams['name'] = name

    req = requests.post('http://localhost:' + str(WasmRuntimePORT) + '/task', 
                    params=queryParams,
                    data=wasmbinary, 
                    headers={"Content-Type":"application/wasm"})
    print(req.text)

def receiveWasmMsg(name, message):
    req = requests.post('http://localhost:' + str(WasmRuntimePORT) + '/task/data', 
                    params={"name":name},
                    data=message, 
                    headers={"Content-Type":"text/plain"})
    print(req.text)

def runModuleFromRunTask(run_task):
    taskID = run_task.task.task_id
    WASMInfo = run_task.task.container.wasm

    # TODO: we would want to constain memory / cpu / etc.

    # Extract the Wasm Bytes
    wasmBytes = WASMInfo.wasm_binary

    # Assemble environment dictionary
    env = {}
    for i in range(len(WASMInfo.environment)):
        envEntry = WASMInfo.environment[i]
        if envEntry.str_value:
            env[key] = envEntry.str_value
        else:
            env[key] = envEntry.value

    # Issue the run task
    runWasmTask(taskID, env, wasmBytes)

def getTaskStatus(task_id):
    req = requests.get('http://localhost:' + str(WasmRuntimePORT) + '/task')
    tasks = req.json() # returns an array of running tasks

    # TODO: How do I check the status of a task in wasmtime?
    if task_id in tasks:
        return messages_pb2.TaskInfo.RUNNING
    else:
        return messages_pb2.TaskInfo.COMPLETED

def killTask(taskID):
    # TODO: implement kill
    pass
