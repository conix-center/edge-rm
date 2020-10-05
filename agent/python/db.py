import pickle

__tasks = {}

def task(taskID):
	return __tasks[taskID]

def set_task(taskID, task):
	__tasks[taskID] = task
	save()

def tasks():
	return __tasks

def save():
	with open('tasks.pickle', 'wb') as file:
		pickle.dump(__tasks, file, protocol=pickle.HIGHEST_PROTOCOL)
	return

def load():
	try:
		with open('tasks.pickle', 'rb') as file:
			__tasks = pickle.load(file)
	except:
		pass