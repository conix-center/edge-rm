import os
import time
import socket
import requests
from tcp_latency import measure_latency

# load environment variables
ENDPOINT = os.getenv('ENDPOINT') or "http://conixdb.com:3001/profile"
AGENTID = os.getenv('AGENT') or "ARM"
TS = os.getenv('TS') or "TEST"
if not os.getenv('DOMAIN0'):
    os.environ['DOMAIN0'] = 'conixdb.com'

time.sleep(1)

print("Starting...")

end = time.time() + 3
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
    values = measure_latency(host=domain, port=80, runs=9)
    sortedValues = sorted(values)
    print(sortedValues)
    pings = pings + str(int(sortedValues[4])) + " "
    domainNum += 1

requestParams = {
    "id": AGENTID,
    "agent": AGENTID,
    "ts":TS,
    "cpu": ctr,
    "pings": pings,
}

requests.get(ENDPOINT, params=requestParams)
