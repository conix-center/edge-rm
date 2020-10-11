#!/usr/bin/env python3
import sys
import argparse
import pydig

from scheduler_library import Framework

def main(host, port, client, sensor, period, func, val):  # pragma: no cover

    #declare a framework
    framework = Framework("Sensor Framework", host, port)

    #print("WASM agents: {}".format(wasm_agents))
    #print("Server agents: {}".format(server_agents))

    # First we need to launch the server task if it's not already running
    agent = framework.getAgentInfoForRunningTask('SensorServer')
    domain = None
    if agent is None:
        #launch the task
        server_agents = framework.findAgents({'domain':None,'cpus':0.5,'mem':100000})
        if len(server_agents) == 0:
            print("Could not find a valid agent for sensor server")
            return


        framework.runTask("SensorServer",server_agents[0][0],
                                {'cpus':0.5,'mem':'100000000'},
                                docker_image='jnoor/coapserver:v1',
                                docker_port_mappings={3002:3002},
                                environment={'SERVER_PORT':'3002'})

        print("Started server task")
        ag = server_agents[0][1]
        for att in ag.attributes:
            if att.name == 'domain':
                domain = att.text.value
    else:
        #extract the domain name
        print("Already running server task")
        for att in agent['attributes']:
            if att['name'] == 'domain':
                domain = att['text']['value']

    # okay now do a dns lookup for this domain
    ip = pydig.query(domain,'A')[0]

    #construct the wasm environment
    env = {}
    env['IP'] = ip
    env['PORT'] = 3002
    env['PERIOD'] = int(args.period)
    if (args.ffunc == "G"):
        env['FILT_FUNC'] = 'G'
        env['FILT_VAL'] = int(args.fval)
    elif args.ffunc == "L":
        env['FILT_FUNC'] = 'L'
        env['FILT_VAL'] = int(args.ffval)

    if args.sensor == "temp" or args.sensor == "press" or args.sensor == "humidity":
        env['SENSOR'] = args.sensor
    else:
        print("Not a valid sensor")
        return

    env['PATH'] = args.client
   
    wasm_agents = framework.findAgents({'executors':'WASM','cpus':1.0,'temperature':'temp'})
    if(len(wasm_agents) == 0):
        print("Could not find wasm agent")
        return

    # Run the WASM task
    wasm_file = open('../wasm/wasm-send/out.wasm','rb')
    runTask("SensorSample",wasm_agents[0][1],{'cpus':1.0},wasm_binary=wasm_file.read(),environment=env)

if __name__ == '__main__':  # pragma: no cover
    parser = argparse.ArgumentParser(description='Launch a CoAP Resource Manager Framework')
    parser.add_argument('--host', required=True, help='the Edge RM Master IP to register with.')
    parser.add_argument('--port', required=False, default=5683, help='the Edge RM Master port to register on.')
    parser.add_argument('--client', help='the client ID that submitted this task')
    parser.add_argument('--sensor', help='The sensor. One of temp,press,humidity')
    parser.add_argument('--period', help='Sample period of the sensor')
    parser.add_argument('--ffunc', help='one of G (>) L (<) or N (none)')
    parser.add_argument('--fval', required = False, help='The value of the filter')
    args = parser.parse_args()
    main(args.host, args.port, args.client, args.sensor, args.period, args.ffunc, args.fval)
