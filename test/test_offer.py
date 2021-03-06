#!/usr/bin/env python3

import pytest
from test import start_master, start_agent
import time
import subprocess
import sys
from edgerm.framework import Framework

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

def test_resource_offer(master, agent):

   time.sleep(10)

   #declare a framework and request offers
   test_framework = Framework('test','127.0.0.1',8080,8080)

   offers = test_framework.getOffers()

   assert len(offers) == 1
   cpu = False
   mem = False
   for r in offers[0].resources:
      if r.name == 'mem':
         assert r.scalar.value == 125000000
         mem = True
      elif r.name == 'cpus':
         assert r.scalar.value == 0.4
         cpu = True

   assert cpu and mem

   #are they still running?
   assert master.poll() is None
   assert agent.poll() is None


def test_repeat_offer(master, agent):

   time.sleep(10)

   #declare a framework and request offers
   test_framework = Framework('test','127.0.0.1',8080,8080)

   offers_1 = test_framework.getOffers()
   offers_2 = test_framework.getOffers()

   assert len(offers_1) == 1
   cpu = False
   mem = False
   for r in offers_1[0].resources:
      if r.name == 'mem':
         assert r.scalar.value == 125000000
         mem = True
      elif r.name == 'cpus':
         assert r.scalar.value == 0.4
         cpu = True

   assert cpu and mem

   assert len(offers_2) == 0

   #are they still running?
   assert master.poll() is None
   assert agent.poll() is None
