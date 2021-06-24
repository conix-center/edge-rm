import requests
import time
import os
import json


cnt = 0
ts = "1624512091"
pathname = './' + str(ts) + '/'

while os.path.exists(pathname + str(cnt) + '-tasks.json'):
	agent_util = dict()
	with open(pathname + str(cnt) + '-tasks.json') as agentfile:
		tasks = json.load(agentfile)
		for task in tasks:
			if task['state'] == 'RUNNING':
				cpu_util = 0
				for i in task['resources']:
					if i['name'] == "cpus":
						cpu_util = i['scalar']['value']
						if task['agentId'] not in agent_util:
							agent_util[task['agentId']] = dict()
							agent_util[task['agentId']]['total'] = 0
						if task['framework']['name'] not in agent_util[task['agentId']]:
							agent_util[task['agentId']][task['framework']['name']] = 0
						agent_util[task['agentId']]["total"] += cpu_util
						agent_util[task['agentId']][task['framework']['name']] += cpu_util
	for agent in agent_util:
		for framework in agent_util[agent]:
			print(', '.join([str(cnt), agent, framework, str(agent_util[agent][framework])]))
	cnt += 1
print("=============")
print("====FRAMEWORK")
print("=============")

sensor_framework = [0] * 300
mapreduce_framework = [0] * 300
total_framework = [0] * 300

cnt = 0
while os.path.exists(pathname + str(cnt) + '-tasks.json'):
	agent_util = dict()
	with open(pathname + str(cnt) + '-tasks.json') as agentfile:
		tasks = json.load(agentfile)
		for task in tasks:
			if task['state'] == 'RUNNING':
				cpu_util = 0
				for i in task['resources']:
					if i['name'] == "cpus":
						cpu_util = i['scalar']['value']
						if task['framework']['name'] not in agent_util:
							agent_util[task['framework']['name']] = 0
						if task['framework']['name'] == 'Map Reduce':
							mapreduce_framework[cnt] += cpu_util
						else:
							print(cnt, len(sensor_framework))
							sensor_framework[cnt] += cpu_util
						total_framework[cnt] += cpu_util
						agent_util[task['framework']['name']] += cpu_util
	for framework in agent_util:
		print(', '.join([str(cnt), framework, str(agent_util[framework])]))
	cnt += 1

print("=============")
print("====TOTAL====")
print("=============")

print("\nMapreduce:" + str(mapreduce_framework))
print("\nSensor:" + str(sensor_framework))
print("\nTotal:" + str(total_framework))

# cnt = 0
# while os.path.exists(pathname + str(cnt) + '-tasks.json'):
# 	agent_util = 0
# 	with open(pathname + str(cnt) + '-tasks.json') as agentfile:
# 		tasks = json.load(agentfile)
# 		for task in tasks:
# 			if task['state'] == 'RUNNING':
# 				cpu_util = 0
# 				for i in task['resources']:
# 					if i['name'] == "cpus":
# 						agent_util += i['scalar']['value']
# 						total[cnt] += i['scalar']['value']
# 	print(', '.join([str(cnt), str(agent_util)]))
# 	cnt += 1

