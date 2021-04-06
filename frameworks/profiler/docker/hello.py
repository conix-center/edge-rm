import os
import time
import socket
import requests
from tcp_latency import measure_latency

class Netcat:

    """ Python 'netcat like' module """

    def __init__(self, ip, port):

        self.buff = ""
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((ip, port))

    def read(self, length = 1024):

        """ Read 1024 bytes off the socket """

        return self.socket.recv(length)
 
    def read_until(self, data):

        """ Read data into the buffer until we have data """

        while not data in self.buff:
            self.buff += self.socket.recv(1024)
 
        pos = self.buff.find(data)
        rval = self.buff[:pos + len(data)]
        self.buff = self.buff[pos + len(data):]
 
        return rval
 
    def write(self, data):

        self.socket.send(data)
    
    def close(self):

        self.socket.close()

# load environment variables
ENDPOINT = os.getenv('ENDPOINT') or "http://conixdb.com:3001/profile"
AGENTID = os.getenv('AGENT') or "1234"
TS = os.getenv('TS') or "1234"
if not os.getenv('DOMAIN0'):
	os.environ['DOMAIN0'] = 'conixdb.com'

time.sleep(1)

print("Starting...")

end = time.time() + 1
ctr = 0

# iterations in X seconds
while time.time() < end:
	ctr += 1

print(ctr)

domainNum = 0
pings = ""
while True:
	domain = os.getenv('DOMAIN' + str(domainNum))
	if not domain:
		break
	print(domain)
	values = measure_latency(host=domain, port=80, runs=5)
	print(values)
	pings = pings + str(int(sum(values) / len(values))) + " "
	domainNum += 1

requestParams = {
	"id": AGENTID,
	"agent": AGENTID,
	"ts":TS,
	"cpu": ctr,
	"pings": pings,
}

requests.get(ENDPOINT, params=requestParams)
