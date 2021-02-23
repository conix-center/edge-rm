import getopt
import socket
import sys
import psutil
import time
import argparse
import uuid
import json
import requests
import hashlib

import messages_pb2

class Framework:

  def __init__(self, framework_name, master, master_port=80, master_api_port=80):
    self.framework_name = framework_name
    b = hashlib.sha256(framework_name.encode('ascii')).digest()
    self.framework_id = str(uuid.UUID(bytes=b[:16]))
    self.master = master
    self.master_port = master_port
    self.master_api_port = master_api_port

  def getTasks(self):
    r = requests.get('http://' + self.master + ':' + str(self.master_api_port) + '/tasks')
    if r.status_code == 200:
      return r.json()

  def getAgents(self):
    r = requests.get('http://' + self.master + ':' + str(self.master_api_port) + '/agents')
    if r.status_code == 200:
      return r.json()

  def getAgentInfoForRunningTask(self, taskName):
    tasks = self.getTasks()
    agents = self.getAgents()

    agent_id_of_running_task = None
    for task in tasks:
      if task['state'] == 'RUNNING' and task['framework']['frameworkId'] == self.framework_id and task['name'] == taskName:
        agent_id_of_running_task = task['agentId']
        return task['agentId']

    return None

  def getAgentProperty(self, agent_id, prop):
    agents = self.getAgents()
    for agent in agents:
      if agent['id'] == agent_id:
        for r in agent['resources']:
          if r['name'] == prop:
            if 'text' in r:
              return r['text']['value']
            if 'scalar' in r:
              return r['scalar']['value']
            if 'device' in r:
              return r['device']['device']

        for att in agent['attributes']:
          if att['name'] == prop:
            if 'text' in att:
              return att['text']['value']
            if 'scalar' in att:
              return att['scalar']['value']
            if 'device' in att:
              return att['device']['device']

    return None


  def getOffers(self):
    # get offers
    wrapper = messages_pb2.WrapperMessage()
    wrapper.type = messages_pb2.WrapperMessage.Type.RESOURCE_REQUEST
    wrapper.request.framework_id = self.framework_id
    response = requests.post("http://" + self.master + ":" + str(self.master_api_port) + '/request', data=wrapper.SerializeToString(), timeout=2, headers={'Content-Type':'application/protobuf'})
    if response:
      wrapper = messages_pb2.WrapperMessage()
      wrapper.ParseFromString(response.content)
      offers = wrapper.offermsg.offers
      return offers
    else:
      raise Exception("Failed to receive resource offers")

  # find agents tries to be smart and filter agents for you
  # offer filters is a dictionary of name:value or name:list of values
  def findAgents(self, offers, offer_filters):
    #print("Find Agents", offer_filters)
    valid_offers = []
    for offer in offers:
      offer_valid = True
      for fkey in offer_filters:
        if fkey == 'agent_name':
          if offer.agent_name != offer_filters[fkey]:
            offer_valid = False
            break
        elif fkey == 'agent_id':
          if offer.agent_id != offer_filters[fkey]:
            offer_valid = False
            break
        else:
          found_resource = False
          for resource in offer.resources:
            if resource.name == fkey:
              if offer_filters[fkey] is None:
                found_resource = True
                break
              elif resource.scalar.value:
                if resource.scalar.value >= offer_filters[fkey]:
                  found_resource = True
                  break
              elif resource.device.device:
                found_resource = True
                break


          for attribute in offer.attributes:
            if attribute.name == fkey:
              if offer_filters[fkey] is None:
                found_resource = True
                break
              elif attribute.text.value:
                if attribute.text.value == offer_filters[fkey] or attribute.text.value in offer_filters[fkey]:
                  found_resource = True
                  break
              elif attribute.set.item:
                if offer_filters[fkey] in attribute.set.item:
                  found_resource = True
                  break

          if found_resource == False:
            offer_valid = False
            break

      if offer_valid:      
        valid_offers.append(offer)

    r = []
    for offer in valid_offers:
      r.append((offer.agent_id,offer))
    return r

  def killTask(self, taskId):
    tasks = self.getTasks()
    for task in tasks:
      if task['taskId'] == taskId and taskId is not None:
        # construct message
        wrapper = messages_pb2.WrapperMessage()
        wrapper.type = messages_pb2.WrapperMessage.Type.KILL_TASK
        wrapper.kill_task.name = task['name']
        wrapper.kill_task.task_id = task['taskId']
        wrapper.kill_task.agent_id = task['agentId']
        wrapper.kill_task.framework.name = task['framework']['name']
        wrapper.kill_task.framework.framework_id = task['framework']['frameworkId']
        print(wrapper)
        request_payload = wrapper.SerializeToString()
        response = requests.post("http://" + self.master + ":" + str(self.master_api_port) + '/kill', data=request_payload, timeout=2, headers={'Content-Type':'application/protobuf'})
        if response:
            wrapper = messages_pb2.WrapperMessage()
            wrapper.ParseFromString(response.content)
            print("Kill task issued!")
            return
        else:
            print("Couldn't issue kill task... ?")
            return

    print("Did not find task")

  def killAllTasks(self):
    tasks = self.getTasks()
    print(tasks)
    for task in tasks:
      if task['state'] != "KILLED":
        print("Kill", task['taskId'])
        self.killTask(task['taskId'])

  def runTask(self, taskName, agent, resources, offer, docker_image=None, wasm_binary=None, docker_port_mappings=None, environment=None):
    # construct message
    wrapper = messages_pb2.WrapperMessage()
    wrapper.type = messages_pb2.WrapperMessage.Type.RUN_TASK

    wrapper.run_task.task.framework.name = self.framework_name
    wrapper.run_task.task.framework.framework_id =self.framework_id
    wrapper.run_task.offer_id = offer.id
    wrapper.run_task.task.name = taskName
    task_id = str(uuid.uuid1())
    wrapper.run_task.task.task_id = task_id
    wrapper.run_task.task.agent_id = agent

    for resource in offer.resources:
        r = wrapper.run_task.task.resources.add()
        r.name = resource.name
        r.type = resource.type
        for res in resources:
          if res == r.name and r.type == messages_pb2.Value.SCALAR:
            r.scalar.value = resources[res]

    ##do something for resources
    #for resource in resources:
    #    name = resource
    #    val = resources[resource]
    #    for r in wrapper.run_task.task.resources:
    #      if r.name == name and r.type == messages_pb2.Value.SCALAR:
    #        r.scalar.value = val

    if docker_image:
      # wrapper.run_task.task.resources.extend(resources_to_use)
      wrapper.run_task.task.container.type = messages_pb2.ContainerInfo.Type.DOCKER
      wrapper.run_task.task.container.docker.image = docker_image
      wrapper.run_task.task.container.docker.network = messages_pb2.ContainerInfo.DockerInfo.Network.HOST
      if docker_port_mappings is not None:
        for host_port, container_port in docker_port_mappings.items():
            port_mapping = wrapper.run_task.task.container.docker.port_mappings.add()
            port_mapping.host_port = host_port
            port_mapping.container_port = container_port

      env = ["{}={}".format(key,environment[key]) for key in environment]
      wrapper.run_task.task.container.docker.environment_variables.extend(env)
    elif wasm_binary:
      wrapper.run_task.task.container.type = messages_pb2.ContainerInfo.Type.WASM
      wrapper.run_task.task.container.wasm.wasm_binary = wasm_binary
      for env in environment:
        e = wrapper.run_task.task.container.wasm.environment.add()
        e.key = env
        if isinstance(environment[env], int):
          e.value = environment[env]
        elif isinstance(environment[env], str):
          e.str_value = environment[env]
        else:
          raise Exception("WASM environment must be of type string or int")
    else:
      raise Exception("Must specify either docker or wasm task")

    response = requests.post("http://" + self.master + ":" + str(self.master_api_port) + '/task', data=wrapper.SerializeToString(), timeout=2, headers={'Content-Type':'application/protobuf'})
    if response:
        wrapper = messages_pb2.WrapperMessage()
        wrapper.ParseFromString(response.content)
        if wrapper.type == messages_pb2.WrapperMessage.Type.ERROR:
            print("Task issuance failed", file=sys.stderr)
            print(wrapper.error.error, file=sys.stderr)
            return None
        else:
            print("Task {} Running!".format(task_id))
            return task_id
    else:
      raise Exception("Failed to receive resource offers")

