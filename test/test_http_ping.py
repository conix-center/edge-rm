#!/usr/bin/env python3

import pytest
from test import start_master, start_agent
import time
import subprocess

@pytest.fixture
def master():
   m = start_master()
   yield m
   m.kill()  

@pytest.fixture
def agent():
   a = start_agent()
   yield a
   a.kill()  

def test_http_ping(master, agent):

   time.sleep(10)

   #are they still running?
   assert master.poll() is None
   assert agent.poll() is None

   #master.stdout.flush()
   #out = master.stdout.read()
   #assert out.find(b'Ping!') != -1
