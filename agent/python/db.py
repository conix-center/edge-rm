import json

__tasks = {}

def task(taskID):
	return __tasks[taskID]

def set_task(taskID, task):
	__tasks[taskID] = task
	save()

def tasks():
	return __tasks

def save():
	with open('tasks.json', 'w') as file:
		file.write(json.dumps(__tasks))
	return

def load():
	try:
		with open('tasks.json', 'r') as file:
			__tasks = json.loads(file)
	except:
		pass