#!/usr/bin/env python
# python 3

import argparse
import getopt
import time
import sys
sys.path.insert(1, '../../CoAPthon')
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

        resources = []
        for resource in wrapper.register_slave.slave.resources:
            resources.append((resource.name, resource.scalar.value))

        # add resource to db
        agent_id = db.add_agent(resources, "coap")
        print(db.get_all())

        # construct response
        wrapper = messages_pb2.WrapperMessage()
        wrapper.slave_registered.slave_id.value = str(agent_id)
        response.payload = wrapper.SerializeToString()
        response.code = defines.Codes.CHANGED.number
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
        print("Ping!")
        wrapper = messages_pb2.WrapperMessage()
        response.payload = wrapper.SerializeToString()
        response.code = defines.Codes.CONTENT.number
        return self, response

    def render_PUT_advanced(self, request, response):
        # self.edit_resource(request)
        return self

    def render_POST_advanced(self, request, response):
        res = self.init_resource(request, PingResource())
        return res

    def render_DELETE_advanced(self, request, response):
        return True

class CoAPServer(CoAP):
    def __init__(self, host, port, multicast=False):
        CoAP.__init__(self, (host, port), multicast)
        self.add_resource('basic/', BasicResource())
        self.add_resource('register/', RegisterResource())
        self.add_resource('ping/', PingResource())

        print ("CoAP Server start on " + host + ":" + str(port))
        print (self.root.dump())

def main(ip, port):  # pragma: no cover
    db.refresh_db()
    multicast = False
    server = CoAPServer(ip, int(port), multicast)
    try:
        server.listen(10)
    except KeyboardInterrupt:
        print("Server Shutdown")
        server.close()
        print("Exiting...")


if __name__ == "__main__":  # pragma: no cover
    if sys.version_info[0] >= 3:
        raise Exception("Must be using Python 2 (yeah...)")
    parser = argparse.ArgumentParser(description='Launch the CoAP Resource Manager Master')
    parser.add_argument('--host', required=True, help='the LAN IP to bind to.')
    parser.add_argument('--port', required=True, help='the local machine port to bind to.')
    args = parser.parse_args()
    main(args.host, args.port)
