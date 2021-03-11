#!/usr/bin/env python3

from test import start_master, start_agent
import time
import subprocess

def test_http_ping():
   m = start_master()
   a = start_agent()

   time.sleep(10)

   #are they still running?
   assert m.poll() is None
   assert a.poll() is None

   #kill
   m.kill()
   a.kill()

   #get the master output
   out, err = m.communicate()
   assert out.find(b'Ping!') != -1
