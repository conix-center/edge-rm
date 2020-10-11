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
sys.path.insert(1, '../support/CoAPthon3')

from coapthon.client.helperclient import HelperClient
from coapthon import defines

import messages_pb2

class Framework:

  def __init__(self, framework_name, master, master_port=5683, master_api_port=80):
    self.framework_name = framework_name
    b = hashlib.sha256(framework_name.encode('ascii')).digest()
    self.framework_id = str(uuid.UUID(bytes=b[:16]))
    self.master = master
    self.master_port = master_port
    self.master_api_port = master_api_port

    #connect to the client
    self.client = HelperClient(server=(master, int(master_port)))

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

    for agent in agents:
      if agent['id'] == agent_id_of_running_task:
        return agent

    return None

  def getOffers(self):
    # get offers
    wrapper = messages_pb2.WrapperMessage()
    wrapper.type = messages_pb2.WrapperMessage.Type.RESOURCE_REQUEST
    wrapper.request.framework_id = self.framework_id
    request_payload = wrapper.SerializeToString()
    ct = {'content_type': defines.Content_types["application/octet-stream"]}
    response = self.client.post('request', request_payload, timeout=2, **ct)
    if response:
      wrapper = messages_pb2.WrapperMessage()
      wrapper.ParseFromString(response.payload)
      offers = wrapper.offermsg.offers
      return offers
    else:
      raise Exception("Failed to receive resource offers")

  # find agents tries to be smart and filter agents for you
  # offer filters is a dictionary of name:value or name:list of values
  def findAgents(self, offer_filters):
    offers = self.getOffers()
   
    valid_offers = []
    for offer in offers:
      offer_valid = True
      for fkey in offer_filters:
        if fkey == 'name':
          if offer.agent_name != name:
            offer_valid = False
            break
        else:
          found_resource = False
          for resource in offer.resources:
            if resource.name == fkey:
              if resource.scalar:
                if resource.scalar.value >= offer_filters[fkey]:
                  found_resource = True
                  break
              if resources.device:
                found_resource = True
                break

          for attribute in offer.attributes:
            if attribute.name == fkey:
              if attribute.text:
                if offer_filters[fkey] is None:
                  found_resource = True
                  break
                elif attribute.text.value == offer_filters[fkey] or attribute.text.value in offer_filters[fkey]:
                  found_resource = True
                  break
              if attribute.set:
                if offer_filters[fkey] is None:
                  found_resource = True
                  break
                elif offer_filters[fkey] in attribute.set.item:
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

  def runTask(self, taskName, agent, resources, docker_image=None, wasm_binary=None, docker_port_mappings=None, environment=None):
    # construct message
    wrapper = messages_pb2.WrapperMessage()
    wrapper.type = messages_pb2.WrapperMessage.Type.RUN_TASK

    wrapper.run_task.task.framework.name = self.framework_name
    wrapper.run_task.task.framework.framework_id =self.framework_id
    wrapper.run_task.task.name = taskName
    task_id = str(uuid.uuid1())
    wrapper.run_task.task.task_id = task_id
    wrapper.run_task.task.agent_id = agent

    #do something for resources
    for resource in resources:
        r = wrapper.run_task.task.resources.add()
        r.name = resource
        val = resources[resource]
        if isinstance(val, int) or isinstance(val, float):
          r.type = messages_pb2.Value.SCALAR
          r.scalar.value = val
        elif isinstance(val, str):
          r.type = messages_pb2.Value.DEVICE

    if docker_image:
      # wrapper.run_task.task.resources.extend(resources_to_use)
      wrapper.run_task.task.container.type = messages_pb2.ContainerInfo.Type.DOCKER
      wrapper.run_task.task.container.docker.image = docker_image
      wrapper.run_task.task.container.docker.network = messages_pb2.ContainerInfo.DockerInfo.Network.HOST
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
        e.key = "PERIOD"
        if isinstance(environment[env], int):
          e.value = environment[key]
        elif isinstance(environment[env], str):
          e.str_value = environment[env]
        else:
          raise Exception("WASM environment must be of type string or int")
    else:
      raise Exception("Must specify either docker or wasm task")

    runtask_payload = wrapper.SerializeToString()
    ct = {'content_type': defines.Content_types["application/octet-stream"]}
    response = self.client.post('task', runtask_payload, timeout=2, **ct)
    if response:
        wrapper = messages_pb2.WrapperMessage()
        wrapper.ParseFromString(response.payload)
        return task_id
    else:
      raise Exception("Failed to receive resource offers")

