import requests
import time
import os

cnt = 0
ts = int(time.time())
pathname = './' + str(ts) + '/'
os.mkdir(pathname)

while True:
	time.sleep(5)
	print(cnt)
	r = requests.get('http://conixdb.com/agents')
	with open(pathname + str(cnt) + '-agents.json', 'w') as outfile:
		outfile.write(r.text)
	r = requests.get('http://conixdb.com/tasks')
	with open(pathname + str(cnt) + '-tasks.json', 'w') as outfile:
		outfile.write(r.text)
	cnt += 1