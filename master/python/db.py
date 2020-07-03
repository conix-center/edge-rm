import messages_pb2
import threading

agents = {}
tasks_to_issue = {}

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

def add_agent(slave):
	aid = str(agent_counter.increment())
	while aid in agents:
		aid = str(agent_counter.increment())
	slave.id.value = aid
	agents[aid] = slave
	tasks_to_issue[aid] = []
	return aid

def schedule_task(runtaskmsg):
	agent_id = runtaskmsg.task.slave_id.value
	tasks_to_issue[agent_id].append(runtaskmsg)

def pop_task(agent_id):
	return tasks_to_issue[agent_id].pop() if tasks_to_issue[agent_id] else None

def get_all_agents():
	return agents.values()

def get_agent(agent_id):
	return agents[agent_id]

def delete_agent(agent_id):
	if agent_id in agents:
		del agents[agent_id]