import json
import messages_pb2
from google.protobuf.json_format import MessageToJson, Parse

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
		if task.state == messages_pb2.TaskInfo.RUNNING or task.state == messages_pb2.TaskInfo.STARTING:
			tasks_to_write[task_id] = MessageToJson(task)
	with open('tasks.json', 'w') as file:
		json.dump(tasks_to_write, file)

def load():
	try:
		tasks_to_read = {}
		with open('tasks.json', 'r') as file:
			tasks_to_read = json.load(file)
			for task_id, task in tasks_to_read.items():
				__tasks[task_id] = messages_pb2.TaskInfo()
				Parse(task, __tasks[task_id])
				print("Loaded tasks", __tasks)
	except:
		print("No tasks file")
		pass