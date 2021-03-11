# Contains basic definitions to start a master process and start an agent process
import subprocess

def start_agent(master_ip='127.0.0.1', master_port=8080):
   l = ['../agent/python/agent.py', '--host', master_ip, '--port', str(master_port), '--config', '../agent/python/config.yaml']
   return subprocess.Popen(l, stdout=subprocess.PIPE)

def start_master(master_ip='127.0.0.1', master_port=5683, master_api_port=8080):
   return subprocess.Popen(['../master/python/master.py','--host',str(master_ip),'--port',str(master_port),'--api-port',str(master_api_port)], stdout=subprocess.PIPE)
