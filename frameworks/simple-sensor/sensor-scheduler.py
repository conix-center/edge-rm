#!/usr/bin/env python3
import sys
import argparse
import pydig
import os

from edgerm.framework import Framework

def main(host, port, client, sensor, period, func, val):  # pragma: no cover

    #declare a framework
    framework = Framework("Sensor Framework", host, port)

    # First we need to launch the server task if it's not already running
    agent = framework.getAgentInfoForRunningTask('SensorServer')

    #Get offers
    offers = framework.getOffers()

    domain = None
    if agent is None:
        #launch the task
        server_agents = framework.findAgents(offers, {'domain':None,'cpus':0.5,'mem':100000000})

        if len(server_agents) == 0:
            print("No available server agents.", file=sys.stderr)
            return


        framework.runTask("SensorServer",server_agents[0],
                                docker_image='jnoor/coapserver:v1',
                                docker_port_mappings={3002:3002},
                                environment={'SERVER_PORT':'3002'})

        print("Started server task on agent: {}".format(server_agents[0].agent_id))

        domain = framework.getAgentProperty(server_agents[0].agent_id,'domain')
    else:
        print("Sensor server task already running.")
        domain = framework.getAgentProperty(agent,'domain')

    # okay now do a dns lookup for this domain
    ip = pydig.query(domain,'A')[0]

    #construct the wasm environment
    env = {}
    env['IP'] = ip
    env['PORT'] = 3002
    env['PERIOD'] = int(args.period)

    #Filters
    if (args.ffunc == "G"):
        env['FILT_FUNC'] = 'G'
        env['FILT_VAL'] = int(args.fval)
    elif args.ffunc == "L":
        env['FILT_FUNC'] = 'L'
        env['FILT_VAL'] = int(args.ffval)

    #Sensor
    sensor_name = None
    if args.sensor == "temperature": 
        env['PATH'] = args.client+'-t'
        env['SENSOR'] = 'temp'
        sensor_name = "temperature_sensor"
    elif args.sensor == "pressure": 
        env['PATH'] = args.client+'-p'
        env['SENSOR'] = 'press'
        sensor_name = "pressure_sensor"
    elif args.sensor == "humidity":
        env['PATH'] = args.client+'-h'
        env['SENSOR'] = 'humidity'
        sensor_name = "humidity_sensor"
    else:
        print("Not a valid sensor.",file=sys.stderr)
        return

    #Find a wasm agent
    wasm_agents = framework.findAgents(offers, {'executors':'WASM','cpus':0.1,sensor_name:None})
    if(len(wasm_agents) == 0):
        print("No available sensor agents.",file=sys.stderr)
        return

    # Run the WASM task

    __location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__)))
    wasm_file = open(os.path.join(__location__,'./wasm/wasm-send/out.wasm'),'rb')
    print("Running task with environment:")
    print(env)
    framework.runTask(args.client + ':' + "SensorSample",wasm_agents[0],wasm_binary=wasm_file.read(),environment=env)
    print("Started sensor task on agent: {}".format(wasm_agents[0].agent_id))

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
