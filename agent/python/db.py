import json
import messages_pb2

__tasks = {}

def task(taskID):
	return __tasks[taskID]

def set_task(taskID, task):
	__tasks[taskID] = task
	save()

def tasks():
	return __tasks

def save():
	tasks_to_write = {}
	for task_id, task in __tasks.items():
		tasks_to_write[task_id] = task.SerializeToString()
	with open('tasks.json', 'wb') as file:
		file.write(json.dumps(__tasks))

def load():
	try:
		tasks_to_read = {}
		with open('tasks.json', 'rb') as file:
			tasks_to_read = json.loads(file)
			for task_id, task in tasks_to_read.items():
				__tasks[task_id] = messages_pb2.TaskInfo()
				__tasks[task_id].ParseFromString(task)

	except:
		pass