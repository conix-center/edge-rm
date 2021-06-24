#!/usr/bin/env python3
import sys
import argparse
import pydig

from edgerm.framework import Framework

def main(host, port):  # pragma: no cover

    #declare a framework
    framework = Framework("DDFlow", host, port)

    # get offers
    offers = framework.getOffers()

    # First we need to launch the server task if it's not already running
    agent = framework.getAgentInfoForRunningTask('DDFlow-Coordinator')

    domain = None
    if agent is None:
        #launch the task
        server_agents = framework.findAgents(offers, {'domain':None,'cpus':0.5,'mem':100000000})

        if len(server_agents) == 0:
            print("No available server agents.", file=sys.stderr)
            return


        framework.runTask("DDFlow-Coordinator",server_agents[0],
                                # {'cpus':0.5,'mem':'100000000'},
                                docker_image='jnoor/ddflow-coordinator:v1',
                                docker_port_mappings={3005:3005},
                                environment={'SERVER_PORT':'3005'})

        print("Started ddflow coordinator task on agent: {}".format(server_agents[0]))

        domain = framework.getAgentProperty(server_agents[0],'domain')
    else:
        print("DDFlow coordinator task already running.")
        domain = framework.getAgentProperty(agent,'domain')

    #Find a device agent
    device_agents = framework.findAgents(offers, {'cpus':1.0, 'mem':100000000})
    if(len(device_agents) == 0):
        print("No available ddflow device agents.",file=sys.stderr)
        return

    # Run the device manager task
    framework.runTask("DDFlow-Device",device_agents[0],
                                # {'cpus':1.0,'mem':'100000000'},
                                docker_image='jnoor/ddflow-device:v1',
                                docker_port_mappings={3000:3000},
                                environment={'SERVER_PORT':'3000'})
    print("Started ddflow device task on agent: {}".format(device_agents[0]))

if __name__ == '__main__':  # pragma: no cover
    parser = argparse.ArgumentParser(description='Launch a CoAP Resource Manager Framework')
    parser.add_argument('--host', required=True, help='the Edge RM Master IP to register with.')
    parser.add_argument('--port', required=False, default=5683, help='the Edge RM Master port to register on.')
    args = parser.parse_args()
    main(args.host, args.port)
