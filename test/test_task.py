#!/usr/bin/env python3

import pytest
from test import start_master, start_agent
import time
import subprocess
import sys
sys.path.insert(0,'../scheduler/')
from scheduler_library import Framework

@pytest.fixture
def master():
   m = start_master()
   yield m
   m.kill()  

@pytest.fixture
def agent():
   a = start_agent(memory='125Mb',cpu=0.4)
   yield a
   a.kill()  

def test_kill_task(master, agent):
   time.sleep(10)

   #declare a framework and request offers
   test_framework = Framework('test','127.0.0.1',8080,8080)

   offers = test_framework.getOffers()
   valid_offers = test_framework.findAgents(offers,{'cpus':0.2})

   #run a docker task on the found agent
   task_id = test_framework.runTask('test', valid_offers[0], valid_offers[0].resources, docker_image='jnoor/dummy:v1')

   time.sleep(5)

   #get the task - there should only be one
   tasks = test_framework.getTasks()
   print(tasks)
   assert len(tasks) == 1
   assert tasks[0]['taskId'] == task_id

   #now wait some time for the task to start running - maybe two minutes
   t = time.time()
   complete = False
   while time.time() - t < 120:
      tasks = test_framework.getTasks()
      print(tasks)
      time.sleep(1)
      if tasks[0]['state'] == 'RUNNING':
         complete = True
         break
   assert complete

   #now kill the task
   test_framework.killTask(task_id)

   #now wait for it to die
   t = time.time()
   complete = False
   while time.time() - t < 120:
      tasks = test_framework.getTasks()
      print(tasks)
      time.sleep(1)
      if tasks[0]['state'] == 'KILLED':
         complete = True
         break
   assert complete


   #are they still running?
   assert master.poll() is None
   assert agent.poll() is None

def test_run_task(master, agent):
   time.sleep(10)

   #declare a framework and request offers
   test_framework = Framework('test','127.0.0.1',8080,8080)

   offers = test_framework.getOffers()
   valid_offers = test_framework.findAgents(offers,{'cpus':0.2})

   #run a docker task on the found agent
   task_id = test_framework.runTask('test', valid_offers[0], valid_offers[0].resources, docker_image='jnoor/dummy:v1')

   time.sleep(5)

   #get the task - there should only be one
   tasks = test_framework.getTasks()
   print(tasks)
   assert len(tasks) == 1
   assert tasks[0]['taskId'] == task_id

   #now wait some time for the task to start running - maybe two minutes
   t = time.time()
   complete = False
   while time.time() - t < 120:
      tasks = test_framework.getTasks()
      print(tasks)
      time.sleep(1)
      if tasks[0]['state'] == 'RUNNING':
         complete = True
         break

   assert complete

   #are they still running?
   assert master.poll() is None
   assert agent.poll() is None
