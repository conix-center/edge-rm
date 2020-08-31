import messages_pb2
import threading
import time
import uuid
from google.protobuf.json_format import MessageToDict

#agents stores the original protobuf
#agentsDict stores extra information about the agent
#both are indexed by agent_id
agents = {}
agentsDict = {}

#same for tasks
tasks = {}
tasksDict = {}

def get_offer_id():
    return str(uuid.uuid4())

def refresh_agent(aid, slave):
    slave.id = aid
    agents[aid] = slave
    if aid not in agentsDict:
        agentsDict[aid] = {}
        agentsDict[aid]['lastPing'] = time.time()*1000
    else:
        agentsDict[aid]['lastPing'] = time.time()*1000

    return aid

def add_task(runtaskmsg):
    task_id = runtaskmsg.task.task_id
    tasks[task_id] = runtaskmsg
    if task_id not in tasksDict:
        tasksDict[task_id] = {}
        tasksDict[task_id]['state'] = 'unissued'
    else:
        tasksDict[task_id]['state'] = 'unissued'

    return task_id

def get_tasks_by_agent(agent_id):
    tasks_by_agent = []
    for task_id, task in tasks.items():
        if task.task.slave_id == agent_id:
            tasks_by_agent.append(task)

    return tasks_by_agent
    
def get_next_unissued_task_by_agent(agent_id):
    for task_id, task in tasks.items():
        if task.task.slave_id == agent_id and tasksDict[task_id]['state'] == 'unissued':
            tasksDict[task_id]['state'] = 'issued'
            return task

def get_all_agents():
    return agents.values()

def get_all_agents_as_dict():
    agents_as_dict = {}
    for agent_id, agent in agents.items():
        agents_as_dict[agent_id] = MessageToDict(agent)
        agents_as_dict[agent_id].update(agentsDict[agent_id])

    return agents_as_dict.values()

def clear_stale_agents():
    agents_to_remove = []
    for agent_id in agents.keys():
        ping_rate = 5000
        if agents[agent_id].ping_rate:
            ping_rate = agents[agent_id].ping_rate
        elapsed = time.time() * 1000 - agentsDict[agent_id]['lastPing']
        if elapsed > ping_rate * 2:
            agents_to_remove.append(agent_id)
    for agent_id in agents_to_remove:
        print("Deleting agent " + str(agent_id))
        del agents[agent_id]
        del agentsDict[agent_id]
