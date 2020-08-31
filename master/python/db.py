import messages_pb2
import threading
import time
import uuid
from google.protobuf.json_format import MessageToDict

agents = {}
tasks_to_issue = {}
last_ping = {}

def get_offer_id():
    return str(uuid.uuid4())

def refresh_agent(aid, slave):
    last_ping[aid] = time.time() * 1000
    if aid not in tasks_to_issue:
        tasks_to_issue[aid] = []
    slave.id = aid
    agents[aid] = MessageToDict(slave)
    agents[aid]['lastPing'] = time.time()*1000
    return aid

def schedule_task(runtaskmsg):
    agent_id = runtaskmsg.task.slave_id
    tasks_to_issue[agent_id].append(runtaskmsg)

def pop_task(agent_id):
    return tasks_to_issue[agent_id].pop() if tasks_to_issue[agent_id] else None

def get_all_agents():
    return agents.values()

def clear_stale_agents():
    agents_to_remove = []
    for agent_id in agents.keys():
        ping_rate = 5000
        if agents[agent_id].ping_rate:
            ping_rate = agents[agent_id].ping_rate
        elapsed = time.time() * 1000 - last_ping[agent_id]
        if elapsed > ping_rate * 2:
            agents_to_remove.append(agent_id)
    for agent_id in agents_to_remove:
        print("Deleting agent " + str(agent_id))
        del agents[agent_id]
