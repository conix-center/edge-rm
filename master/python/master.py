#!/usr/bin/env python3

import argparse
import getopt
import time
import sys
import threading

import flask
app = flask.Flask(__name__)

sys.path.insert(1, '../../support/CoAPthon3')
from coapthon.server.coap import CoAP
from coapthon import defines
from coapthon.resources.resource import Resource

import messages_pb2
import db

offer_timeout = None

# TODO + NOTES:
# Issue: What if the agent never pings to receive their task?
# Issue: How do we ensure that the agent successfully started the task we assigned them?
# Todo: De-register agent when they dont ping for a while
# Todo: Keep track of available resources

lock = threading.Lock()

def GetResourceOffer(wrapper):
    lock.acquire()
    framework_id = wrapper.request.framework_id

    #first, clear any agents that have dropped off
    db.clear_stale_agents()

    #construct resource offer subtracting the resources of any tasks in the task queue
    print("\nGot resource offer request! Framework \"" + framework_id + "\"\n")

    #currently just giving the framework everything we got
    wrapper = messages_pb2.WrapperMessage()
    wrapper.type = messages_pb2.WrapperMessage.Type.RESOURCE_OFFER
    wrapper.offermsg.framework_id = framework_id
    for agent in db.get_all_agents():
        offer = messages_pb2.Offer()
        offer.id = db.get_offer_id()
        offer.framework_id = framework_id
        offer.agent_id = agent.id
        offer.attributes.extend(agent.attributes)
        offer.resources.extend(agent.resources)

        #What are free resources?
            # The amount of resources that are free
            # plus the amount of resources currently being used <- how do we collect this one? for now ignore
            # minus the amount of resources dedicated running or pending tasks
            # minus the amount of resources that are currently in valid offers

        #get pending tasks for agent to subract pending resources
        pending_tasks = db.get_all_pending_tasks_by_agent(agent.id)

        #currently only do subtraction for scalars
        for resource in offer.resources:
            for task in pending_tasks:
                for tresource in task.resources:
                    if resource.name == tresource.name:
                        if resource.scalar.value and tresource.scalar.value:
                            resource.scalar.value -= tresource.scalar.value

        pending_offers = db.get_offers_by_agent_id(agent.id)
        
        for resource in offer.resources:
            for off in pending_offers:
                if time.time() < off.expiration_time:
                    for tresource in off.resources:
                        if resource.name == tresource.name:
                            if resource.scalar.value and tresource.scalar.value:
                                resource.scalar.value -= tresource.scalar.value
       
        removal_indexes = []
        for i, r in enumerate(offer.resources):
            if r.type == messages_pb2.Value.Type.SCALAR and r.scalar.value is not None and r.scalar.value <= 0.001:
                removal_indexes.append(i)

        #removal_list
        for index in sorted(removal_indexes, reverse=True):
            del offer.resources[index]

        #set the offer time
        offer.offer_time = time.time()

        #set the offer expiration
        offer.expiration_time = time.time() + offer_timeout

        #add the offer to the db
        db.add_offer(offer.id, agent.id, offer)
        
        if len(offer.resources) > 0:
            wrapper.offermsg.offers.append(offer)

    lock.release()
    return wrapper.SerializeToString()

def RunTask(wrapper):
    lock.acquire()
    # print request (do nothing right now)
    print("    Framework Name: " + wrapper.run_task.task.framework.name)
    print("    Framework ID:   " + wrapper.run_task.task.framework.framework_id)
    print("    Task Name:      " + wrapper.run_task.task.name)
    print("    Task ID:        " + wrapper.run_task.task.task_id)
    print("    Selected Agent: " + wrapper.run_task.task.agent_id)
    for i in range(len(wrapper.run_task.task.resources)):
        resource = wrapper.run_task.task.resources[i]
        print("        Resource: (" + resource.name + ") type: " + str(resource.type) + " amt: " + str(resource.scalar).strip())

    # Verify that the RunTaskRequest is on an unexpired offer of the right resources and 
    try:
        offer = db.get_offer_by_offer_id(wrapper.run_task.offer_id)
    except ValueError:
        print("Not a valid offer ID")
        wrapper = messages_pb2.WrapperMessage()
        wrapper.type = messages_pb2.WrapperMessage.Type.ERROR
        wrapper.error.error = "Not a valid offer ID"
        lock.release()
        return wrapper.SerializeToString()

    #unexpired
    if time.time() > offer.expiration_time:
        print("Offer expired")
        wrapper = messages_pb2.WrapperMessage()
        wrapper.type = messages_pb2.WrapperMessage.Type.ERROR
        wrapper.error.error = "Offer Expired"
        lock.release()
        return wrapper.SerializeToString()

    #check for valid resources - just scalars for now
    for tresource in wrapper.run_task.task.resources:
        r = {x.name: (x.type, x.scalar,x.ranges,x.set,x.text,x.device) for x in offer.resources}
        if tresource.name not in r:
            print ("Resource not in offer")
            wrapper = messages_pb2.WrapperMessage()
            wrapper.type = messages_pb2.WrapperMessage.Type.ERROR
            wrapper.error.error = "Resource not in offer"
            lock.release()
            return wrapper.SerializeToString()
        elif tresource.type != r[tresource.name][0]:
            print ("Resource type doesn't match offer")
            wrapper = messages_pb2.WrapperMessage()
            wrapper.type = messages_pb2.WrapperMessage.Type.ERROR
            wrapper.error.error = "Resource type doesn't match offer"
            lock.release()
            return wrapper.SerializeToString()
        elif tresource.scalar.value and r[tresource.name][1].value and tresource.scalar.value > r[tresource.name][1].value:
            print ("Resource value exceeds offer")
            wrapper = messages_pb2.WrapperMessage()
            wrapper.type = messages_pb2.WrapperMessage.Type.ERROR
            wrapper.error.error = "Resource value exceeds offer"
            lock.release()
            return wrapper.SerializeToString()
            
    # TODO: Forward the request onto the particular device through a ping/pong
    db.add_task(wrapper.run_task)

    # construct response
    wrapper = messages_pb2.WrapperMessage()
    wrapper.type = messages_pb2.WrapperMessage.Type.PONG
    wrapper.pong.agent_id = wrapper.run_task.task.agent_id
    lock.release()
    return wrapper.SerializeToString()

def KillTask(wrapper):
    lock.acquire()
    # print request (do nothing right now)
    print("    Framework Name: " + wrapper.kill_task.framework.name)
    print("    Framework ID:   " + wrapper.kill_task.framework.framework_id)
    print("    Task Name:      " + wrapper.kill_task.name)
    print("    Task ID:        " + wrapper.kill_task.task_id)
    print("    Selected Agent: " + wrapper.kill_task.agent_id)

    # TODO: Forward the request onto the particular device through a ping/pong
    db.add_kill_task(wrapper.kill_task)

    # construct response
    wrapper = messages_pb2.WrapperMessage()
    wrapper.type = messages_pb2.WrapperMessage.Type.PONG
    wrapper.pong.agent_id = wrapper.kill_task.agent_id
    lock.release()
    return wrapper.SerializeToString()

def GetPing(wrapper):
    lock.acquire()
    agent_id = wrapper.ping.agent.id
    agent_name = wrapper.ping.agent.name
    if not agent_id:
        return self
    print("Ping! Agent ID:(" + str(agent_id) + ") Name:(" + str(agent_name) + ")")

    #refresh the agent timing
    db.refresh_agent(agent_id, wrapper.ping.agent)

    #update the state of any tasks it may have sent
    db.refresh_tasks(wrapper.ping.tasks)

    task_to_run = db.get_next_unissued_task_by_agent(agent_id)
    task_to_kill = db.get_next_unissued_kill_by_agent(agent_id)

    # construct response
    wrapper = messages_pb2.WrapperMessage()
    wrapper.type = messages_pb2.WrapperMessage.Type.PONG
    wrapper.pong.agent_id = str(agent_id)
    if task_to_run:
        print("Got a task to schedule!!!")
        wrapper.pong.run_task.task.CopyFrom(task_to_run)
    if task_to_kill:
        print("Got a task to kill!")
        wrapper.pong.kill_task.CopyFrom(task_to_kill)
    lock.release()
    return wrapper.SerializeToString()

class RequestOfferResource(Resource):
    def __init__(self, name="RequestResource", coap_server=None):
        super(RequestOfferResource, self).__init__(name, coap_server, visible=True,
                                            observable=True, allow_children=True)
        self.payload = "Test"
        self.resource_type = "rt1"
        self.content_type = "text/plain"
        self.interface_type = "if1"

    def render_POST_advanced(self, request, response):
        wrapper = messages_pb2.WrapperMessage()
        try:
            wrapper.ParseFromString(request.payload)
        except:
            print("Error parsing protobuf - not responding.")
            response.code = defines.Codes.BAD_REQUEST.number
            return self, response

        response.payload = GetResourceOffer(wrapper)

        response.code = defines.Codes.CHANGED.number
        response.content_type = defines.Content_types["application/octet-stream"]
        return self, response

class RunTaskResource(Resource):
    def __init__(self, name="RunTaskResource", coap_server=None):
        super(RunTaskResource, self).__init__(name, coap_server, visible=True,
                                            observable=True, allow_children=True)
        self.payload = "Test"
        self.resource_type = "rt1"
        self.content_type = "text/plain"
        self.interface_type = "if1"

    def render_POST_advanced(self, request, response):
        print("Received Task Request!")

        # unpack request
        wrapper = messages_pb2.WrapperMessage()
        try:
            wrapper.ParseFromString(request.payload)
        except:
            print("Error parsing protobuf - not responding.")
            response.code = defines.Codes.BAD_REQUEST.number
            return self, response

        response.payload = RunTask(wrapper)

        response.code = defines.Codes.CHANGED.number
        response.content_type = defines.Content_types["application/octet-stream"]
        return self, response

class KillTaskResource(Resource):
    def __init__(self, name="KillTaskResource", coap_server=None):
        super(KillTaskResource, self).__init__(name, coap_server, visible=True,
                                            observable=True, allow_children=True)
        self.payload = "Test"
        self.resource_type = "rt1"
        self.content_type = "text/plain"
        self.interface_type = "if1"

    def render_POST_advanced(self, request, response):
        print("Received Kill Task!")

        # unpack request
        wrapper = messages_pb2.WrapperMessage()
        try:
            wrapper.ParseFromString(request.payload)
        except:
            print("Error parsing protobuf - not responding.")
            response.code = defines.Codes.BAD_REQUEST.number
            return self, response

        # construct response
        wrapper = KillTask(wrapper)

        wrapper.type = messages_pb2.WrapperMessage.Type.PONG
        wrapper.pong.agent_id = wrapper.kill_task.agent_id
        response.payload = wrapper.SerializeToString()
        response.code = defines.Codes.CHANGED.number
        response.content_type = defines.Content_types["application/octet-stream"]
        return self, response


class PingResource(Resource):
    def __init__(self, name="PingResource", coap_server=None):
        super(PingResource, self).__init__(name, coap_server, visible=True,
                                            observable=True, allow_children=True)
        self.payload = "Hello World"
        self.resource_type = "rt1"
        self.content_type = "text/plain"
        self.interface_type = "if1"

    def render_POST_advanced(self, request, response):
        #unpack request
        
        print("Recevied CoAP ping request: " + str(request))

        wrapper = messages_pb2.WrapperMessage()

        try:
            wrapper.ParseFromString(request.payload)
        except:
            print("Error parsing protobuf - not responding.")
            response.code = defines.Codes.BAD_REQUEST.number
            return self, response

        response.payload = GetPing(wrapper)

        response.code = defines.Codes.CONTENT.number
        # response.code = defines.Codes.CHANGED.number
        response.content_type = defines.Content_types["application/octet-stream"]
        return self, response

class CoAPServer(CoAP):
    def __init__(self, host, port, multicast=False):
        CoAP.__init__(self, (host, port), multicast)
        self.add_resource('request/', RequestOfferResource())
        self.add_resource('task/', RunTaskResource())
        self.add_resource('kill/', KillTaskResource())
        self.add_resource('ping/', PingResource())

        print ("CoAP Server start on " + host + ":" + str(port))
        print (self.root.dump())

def start_coap_server(ip, port):  # pragma: no cover
    multicast = False
    server = CoAPServer(ip, int(port), multicast)
    try:
        server.listen(10)
    except KeyboardInterrupt:
        print("Server Shutdown")
        server.close()
        sys.exit()
    except:
        print("Server Shutdown")
        server.close()
        print("Restarting...")
        start_coap_server(ip, port)


### This section is responsible for the HTTP server
@app.route('/agents', methods=['GET'])
@app.route('/', methods=['GET'])
def get_agents():
    return flask.jsonify(list(db.get_all_agents_as_dict()))

@app.route('/frameworks', methods=['GET'])
def get_frameworks():
    return flask.jsonify(list(db.get_all_frameworks_as_dict()))

@app.route('/tasks', methods=['GET'])
def get_tasks():
    return flask.jsonify(list(db.get_all_tasks_as_dict()))

@app.route('/request', methods=['POST'])
def post_request_resource_offer():
    wrapper = messages_pb2.WrapperMessage()
    try:
        wrapper.ParseFromString(flask.request.get_data())
    except:
        print("Error parsing protobuf.")
        return "Error parsing protobuf."
    return GetResourceOffer(wrapper)

@app.route('/task', methods=['POST'])
def post_run_task():
    wrapper = messages_pb2.WrapperMessage()
    try:
        wrapper.ParseFromString(flask.request.get_data())
    except:
        print("Error parsing protobuf.")
        return "Error parsing protobuf."
    return RunTask(wrapper)

@app.route('/kill', methods=['POST'])
def post_kill_task():
    wrapper = messages_pb2.WrapperMessage()
    try:
        wrapper.ParseFromString(flask.request.get_data())
    except:
        print("Error parsing protobuf.")
        return "Error parsing protobuf."
    return KillTask(wrapper)

@app.route('/ping', methods=['POST'])
def post_ping():
    wrapper = messages_pb2.WrapperMessage()
    try:
        wrapper.ParseFromString(flask.request.get_data())
    except:
        print("Error parsing protobuf.")
        return "Error parsing protobuf."
    return GetPing(wrapper)

def start_api_server(host, port):
    app.run(host=host,port=port)


if __name__ == "__main__":  # pragma: no cover
    if sys.version_info[0] < 3:
        raise Exception("Must be using Python 3")
    parser = argparse.ArgumentParser(description='Launch the CoAP Resource Manager Master')
    parser.add_argument('--host', required=True, help='the LAN IP to bind to.')
    parser.add_argument('--port', required=False, default=5683, help='the local machine port to bind to.')
    parser.add_argument('--api-port', required=False, default=8080, help='the local machine port to bind to.')
    parser.add_argument('--offer-timeout', required=False, default=10, help='Seconds after offer until the offer expires')
    args = parser.parse_args()

    offer_timeout = args.offer_timeout

    #start API server in a thread
    api_server_thread = threading.Thread(target=start_api_server,args=(args.host,args.api_port,), daemon = True)
    api_server_thread.start()

    #start coap server
    start_coap_server(args.host, args.port)
