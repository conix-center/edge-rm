#!/usr/bin/env python3

from test import start_master, start_agent
import time
import subprocess
import sys
sys.path.insert(0,'../scheduler/')
from scheduler_library import Framework

def test_run_task():
   m = start_master()

   #start agent with specific cpu and memory
   a = start_agent(memory='125Mb',cpu=0.4)

   time.sleep(10)

   #declare a framework and request offers
   test_framework = Framework('test','127.0.0.1',8080,8080)

   offers = test_framework.getOffers()
   agents = test_framework.findAgents(offers,{'cpus':0.2})

   #run a docker task on the found agent
   task_id = test_framework.runTask('test',agents[0][0], agents[0][1].resources, agents[0][1],docker_image='jnoor/dummy:v1')

   time.sleep(5)

   #get the task - there should only be one
   tasks = test_framework.getTasks()
   assert len(tasks) == 1
   assert tasks[0]['taskId'] == task_id

   #now wait some time for the task to start running - maybe two minutes
   t = time.time()
   complete = False
   while time.time() - t < 120:
      tasks = test_framework.getTasks()
      time.sleep(1)
      if tasks[0]['state'] == 'COMPLETE':
         complete = True
         break

   assert complete

   #are they still running?
   assert m.poll() is None
   assert a.poll() is None

   #kill
   m.kill()
   a.kill()
