#!/usr/bin/env python3

import argparse
import getopt
import time
import sys
sys.path.insert(1, '../../CoAPthon3')
from coapthon.server.coap import CoAP
from coapthon import defines
from coapthon.resources.resource import Resource

import messages_pb2

import db

# TODO + NOTES:
# Issue: What if the slave never pings to receive their task?
# Issue: How do we ensure that the slave successfully started the task we assigned them?
# Todo: De-register agent when they dont ping for a while
# Todo: Keep track of available resources

class BasicResource(Resource):
    def __init__(self, name="BasicResource", coap_server=None):
        super(BasicResource, self).__init__(name, coap_server, visible=True,
                                            observable=True, allow_children=True)
        self.payload = "Hello World"
        self.resource_type = "rt1"
        self.content_type = "text/plain"
        self.interface_type = "if1"

    def render_GET(self, request):
        return self

    def render_PUT(self, request):
        self.edit_resource(request)
        return self

    def render_POST(self, request):
        res = self.init_resource(request, BasicResource())
        return res

    def render_DELETE(self, request):
        return True

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
        wrapper.ParseFromString(request.payload)
        framework_id = wrapper.request.framework_id

        #first, clear any agents that have dropped off
        db.clear_stale_agents()

        #construct resource offer
        print("\nGot resource offer request! Framework \"" + framework_id + "\"\n")
        #currently just giving the framework everything we got
        wrapper = messages_pb2.WrapperMessage()
        wrapper.offermsg.framework_id = framework_id
        for agent in db.get_all_agents():
            offer = wrapper.offermsg.offers.add()
            offer.id = db.get_offer_id()
            offer.framework_id = framework_id
            offer.slave_id = agent.id
            offer.resources.extend(agent.resources)
            offer.attributes.extend(agent.attributes)
        response.payload = wrapper.SerializeToString()
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
        wrapper.ParseFromString(request.payload)

        # print request (do nothing right now)
        print("    Framework Name: " + wrapper.run_task.framework.name)
        print("    Framework ID:   " + wrapper.run_task.framework.framework_id)
        print("    Task Name:      " + wrapper.run_task.task.name)
        print("    Task ID:        " + wrapper.run_task.task.task_id)
        print("    Selected Slave: " + wrapper.run_task.task.slave_id)
        for i in range(len(wrapper.run_task.task.resources)):
            resource = wrapper.run_task.task.resources[i]
            print("        Resource: (" + resource.name + ") type: " + str(resource.type) + " amt: " + str(resource.scalar).strip())

        # TODO: Forward the request onto the particular device through a ping/pong
        db.schedule_task(wrapper.run_task)

        # construct response
        wrapper = messages_pb2.WrapperMessage()
        wrapper.pong.slave_id = "1234"
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
        wrapper = messages_pb2.WrapperMessage()
        wrapper.ParseFromString(request.payload)

        agent_id = wrapper.ping.slave.id
        if not agent_id:
            return self
        print("Ping! Agent (" + str(agent_id) + ")")
        db.refresh_agent(agent_id, wrapper.ping.slave)
        task_to_run = db.pop_task(agent_id)

        # construct response
        wrapper = messages_pb2.WrapperMessage()
        if task_to_run:
            print("Got a task to schedule!!!")
            wrapper.run_task.CopyFrom(task_to_run)
        else:
            wrapper.pong.slave_id = str(agent_id)
        response.payload = wrapper.SerializeToString()
        response.code = defines.Codes.CONTENT.number
        # response.code = defines.Codes.CHANGED.number
        response.content_type = defines.Content_types["application/octet-stream"]
        return self, response

class CoAPServer(CoAP):
    def __init__(self, host, port, multicast=False):
        CoAP.__init__(self, (host, port), multicast)
        self.add_resource('basic/', BasicResource())
        # self.add_resource('register/', RegisterResource())
        self.add_resource('request/', RequestOfferResource())
        self.add_resource('task/', RunTaskResource())
        self.add_resource('ping/', PingResource())

        print ("CoAP Server start on " + host + ":" + str(port))
        print (self.root.dump())

def main(ip, port):  # pragma: no cover
    multicast = False
    server = CoAPServer(ip, int(port), multicast)
    try:
        server.listen(10)
    except:
        print("Server Shutdown")
        server.close()
        print("Exiting...")


if __name__ == "__main__":  # pragma: no cover
    # if sys.version_info[0] >= 3:
    #     raise Exception("Must be using Python 2 (yeah...)")
    parser = argparse.ArgumentParser(description='Launch the CoAP Resource Manager Master')
    parser.add_argument('--host', required=True, help='the LAN IP to bind to.')
    parser.add_argument('--port', required=False, default=5683, help='the local machine port to bind to.')
    args = parser.parse_args()
    main(args.host, args.port)
