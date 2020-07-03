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

class RegisterResource(Resource):
    def __init__(self, name="RegisterResource", coap_server=None):
        super(RegisterResource, self).__init__(name, coap_server, visible=True,
                                            observable=True, allow_children=True)
        self.payload = "Test"
        self.resource_type = "rt1"
        self.content_type = "text/plain"
        self.interface_type = "if1"

    def render_GET_advanced(self, request, response):
        return self

    def render_PUT_advanced(self, request, response):
        return self

    def render_POST_advanced(self, request, response):
        print("Registering new agent!")

        # unpack request
        wrapper = messages_pb2.WrapperMessage()
        wrapper.ParseFromString(request.payload)

        # add resource to db
        if not wrapper.register_slave.slave:
            return self
        agent_id = db.add_agent(wrapper.register_slave.slave)
        print(db.get_all_agents())

        # construct response
        wrapper = messages_pb2.WrapperMessage()
        wrapper.slave_registered.slave_id.value = str(agent_id)
        response.payload = wrapper.SerializeToString()
        response.code = defines.Codes.CHANGED.number
        response.content_type = defines.Content_types["application/octet-stream"]
        return self, response

    def render_DELETE_advanced(self, request, response):
        return True

class RequestResource(Resource):
    def __init__(self, name="RequestResource", coap_server=None):
        super(RequestResource, self).__init__(name, coap_server, visible=True,
                                            observable=True, allow_children=True)
        self.payload = "Test"
        self.resource_type = "rt1"
        self.content_type = "text/plain"
        self.interface_type = "if1"

    def render_GET_advanced(self, request, response):
        return self

    def render_PUT_advanced(self, request, response):
        return self

    def render_POST_advanced(self, request, response):
        wrapper = messages_pb2.WrapperMessage()
        wrapper.ParseFromString(request.payload)
        framework_id = wrapper.request.framework_id.value
        print("Got resource request from Framework \"" + framework_id + "\"")

        #construct resource offer
        #currently just giving the framework everything we got
        wrapper = messages_pb2.WrapperMessage()
        wrapper.offer.framework_id.value = framework_id
        for agent in db.get_all_agents():
            offer = wrapper.offer.offers.add()
            offer.id.value = db.get_offer_id()
            offer.framework_id.value = framework_id
            offer.slave_id.value = agent.id
            for resrc in agent.resources:
                resource = offer.resources.add()
                resource.name = resrc[0]
                resource.type = messages_pb2.Value.SCALAR
                resource.scalar.value = resrc[1]
            for attrib in agent.attributes:
                attribute = offer.attributes.add()
                attribute.name = attrib[0]
                attribute.type = messages_pb2.Value.SCALAR
                attribute.scalar.value = attrib[1]
        response.payload = wrapper.SerializeToString()
        response.code = defines.Codes.CHANGED.number
        response.content_type = defines.Content_types["application/octet-stream"]
        return self, response

    def render_DELETE_advanced(self, request, response):
        return True

class RunTaskResource(Resource):
    def __init__(self, name="RunTaskResource", coap_server=None):
        super(RunTaskResource, self).__init__(name, coap_server, visible=True,
                                            observable=True, allow_children=True)
        self.payload = "Test"
        self.resource_type = "rt1"
        self.content_type = "text/plain"
        self.interface_type = "if1"

    def render_GET_advanced(self, request, response):
        return self

    def render_PUT_advanced(self, request, response):
        return self

    def render_POST_advanced(self, request, response):
        print("Received Task Request!")

        # unpack request
        wrapper = messages_pb2.WrapperMessage()
        wrapper.ParseFromString(request.payload)

        # print request (do nothing right now)
        print("    Framework Name: " + wrapper.run_task.framework.name)
        print("    Framework ID:   " + wrapper.run_task.framework.framework_id.value)

        # TODO: Forward the request onto the particular device through a ping/pong

        # construct response
        wrapper = messages_pb2.WrapperMessage()
        wrapper.ping.slave_id.value = "1234"
        response.payload = wrapper.SerializeToString()
        response.code = defines.Codes.CHANGED.number
        response.content_type = defines.Content_types["application/octet-stream"]
        return self, response

    def render_DELETE_advanced(self, request, response):
        return True

class PingResource(Resource):
    def __init__(self, name="PingResource", coap_server=None):
        super(PingResource, self).__init__(name, coap_server, visible=True,
                                            observable=True, allow_children=True)
        self.payload = "Hello World"
        self.resource_type = "rt1"
        self.content_type = "text/plain"
        self.interface_type = "if1"

    def render_GET_advanced(self, request, response):
        return self

    def render_PUT_advanced(self, request, response):
        # self.edit_resource(request)
        return self

    def render_POST_advanced(self, request, response):
        # res = self.init_resource(request, PingResource())

        #unpack request
        wrapper = messages_pb2.WrapperMessage()
        wrapper.ParseFromString(request.payload)

        agent_id = wrapper.ping.slave_id.value
        print("Ping! Agent (" + str(agent_id) + ")")

        # construct response
        wrapper = messages_pb2.WrapperMessage()
        response.payload = wrapper.SerializeToString()
        print(response.payload)
        response.code = defines.Codes.CONTENT.number
        response.content_type = defines.Content_types["application/octet-stream"]
        return self, response

    def render_DELETE_advanced(self, request, response):
        return True

class CoAPServer(CoAP):
    def __init__(self, host, port, multicast=False):
        CoAP.__init__(self, (host, port), multicast)
        self.add_resource('basic/', BasicResource())
        self.add_resource('register/', RegisterResource())
        self.add_resource('request/', RequestResource())
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
    parser.add_argument('--port', required=True, help='the local machine port to bind to.')
    args = parser.parse_args()
    main(args.host, args.port)
