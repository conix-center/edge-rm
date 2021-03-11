# Contains basic definitions to start a master process and start an agent process
import subprocess
import yaml


def start_agent(master_ip='127.0.0.1', master_port=8080, memory='500Mb', cpu=1.0):
   start_agent.counter += 1
   #get the config file in the other folder
   f = open('../agent/python/config.yaml')
   config = yaml.load(f, Loader=yaml.FullLoader)
   config['maxMem'] = memory
   config['maxCPUs'] = cpu

   cname = './config-'+ str(start_agent.counter) +'.yaml'
   wf = open(cname,'w')
   yaml.dump(config,wf)

   l = ['../agent/python/agent.py', '--host', master_ip, '--port', str(master_port), '--config', cname]
   return subprocess.Popen(l, stdout=subprocess.PIPE)
start_agent.counter = 0

def start_master(master_ip='127.0.0.1', master_port=5683, master_api_port=8080):
   return subprocess.Popen(['../master/python/master.py','--host',str(master_ip),'--port',str(master_port),'--api-port',str(master_api_port)], stdout=subprocess.PIPE)
