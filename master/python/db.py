import messages_pb2
import threading
import time

agents = {}
tasks_to_issue = {}
last_ping = {}

class AtomicCounter:
    def __init__(self, initial=0):
        """Initialize a new atomic counter to given initial value (default 0)."""
        self.value = initial
        self._lock = threading.Lock()

    def increment(self, num=1):
        """Atomically increment the counter by num (default 1) and return the
        new value.
        """
        with self._lock:
            self.value += num
            return self.value
agent_counter = AtomicCounter()
offer_counter = AtomicCounter()

def get_offer_id():
    return str(offer_counter.increment())

def refresh_agent(aid, slave):
    last_ping[aid] = time.time() * 1000
    if aid not in tasks_to_issue:
        tasks_to_issue[aid] = []
    slave.id = aid
    agents[aid] = slave
    return aid

def add_agent(slave):
    aid = str(agent_counter.increment())
    while aid in agents:
        aid = str(agent_counter.increment())
    tasks_to_issue[aid] = []
    return refresh_agent(aid, slave)

def schedule_task(runtaskmsg):
    agent_id = runtaskmsg.task.slave_id
    tasks_to_issue[agent_id].append(runtaskmsg)

def pop_task(agent_id):
    return tasks_to_issue[agent_id].pop() if tasks_to_issue[agent_id] else None

def get_all_agents():
    return agents.values()

def get_agent(agent_id):
    return agents[agent_id]

def delete_agent(agent_id):
    print("Deleting agent " + str(agent_id))
    if agent_id in agents:
        del agents[agent_id]

def has_agent(agent_id):
    return agent_id in agents

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
        delete_agent(agent_id)