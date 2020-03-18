
#!/usr/bin/env python

import getopt
import time
import sys
sys.path.insert(1, './CoAPthon')
from coapthon.server.coap import CoAP

from coapthon import defines
from coapthon.resources.resource import Resource

import messages_pb2

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

        self.counter = 0
        self.slaves = dict()

    def render_GET(self, request):
        return self

    def render_PUT(self, request):
        return self

    def render_POST(self, request):
        print("Registering new agent!")
        wrapper = messages_pb2.WrapperMessage()
        wrapper.ParseFromString(request.payload)
        self.counter += 1
        agent = {"id": self.counter}
        for resource in wrapper.register_slave.slave.resources:
            agent[resource.name] = resource.scalar.value
        self.slaves[agent["id"]] = agent
        print("Num Slaves:", len(agent.keys()))
        print("Slaves:", self.slaves)
        return self

    def render_DELETE(self, request):
        return True

class CoAPServer(CoAP):
    def __init__(self, host, port, multicast=False):
        CoAP.__init__(self, (host, port), multicast)
        self.add_resource('basic/', BasicResource())
        self.add_resource('register/', RegisterResource())

        print "CoAP Server start on " + host + ":" + str(port)
        print self.root.dump()


def usage():  # pragma: no cover
    print "coapserver.py -i <ip address> -p <port>"


def main(argv):  # pragma: no cover
    ip = "0.0.0.0"
    port = 5683
    multicast = False
    try:
        opts, args = getopt.getopt(argv, "hi:p:m", ["ip=", "port=", "multicast"])
    except getopt.GetoptError:
        usage()
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            usage()
            sys.exit()
        elif opt in ("-i", "--ip"):
            ip = arg
        elif opt in ("-p", "--port"):
            port = int(arg)
        elif opt in ("-m", "--multicast"):
            multicast = True

    server = CoAPServer(ip, port, multicast)
    try:
        server.listen(10)
    except KeyboardInterrupt:
        print "Server Shutdown"
        server.close()
        print "Exiting..."


if __name__ == "__main__":  # pragma: no cover
    main(sys.argv[1:])
