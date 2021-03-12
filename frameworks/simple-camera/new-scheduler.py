#!/usr/bin/env python3
import sys
import argparse
from random import randint

from edgerm.framework import Framework

def main(host, port, client, camera):  # pragma: no cover

    #declare a framework
    framework = Framework("Camera Framework", host, port)

    # First we need to launch the server task if it's not already running
    agent = framework.getAgentInfoForRunningTask('HTTP endpoint')

    #Get offers
    offers = framework.getOffers()

    #Find a classify agent
    classify_agents = framework.findAgents(offers, {'cpus':1.0,"mem":200000000})
    if(len(classify_agents) == 0):
        print("No available classify agents.",file=sys.stderr)
        return

    #Find a camera agent
    cam_agents = framework.findAgents(offers, {'cpus':0.5,"mem":100000000,"picam":None,'agent_id':camera})
    if(len(cam_agents) == 0):
        print("No available camera agents.",file=sys.stderr)
        return

    domain = None
    if agent is None:
        #launch the task
        server_agents = framework.findAgents(offers, {'domain':None,'cpus':0.5,'mem':100000000})

        if len(server_agents) == 0:
            print("No available server agents.", file=sys.stderr)
            return

        framework.runTask("HTTP endpoint", server_agents[0],
                                docker_image='jnoor/hellocameraserver:v1',
                                docker_port_mappings={3003:3003},
                                environment={'SERVER_PORT':'3003'})

        print("Started server task on agent: {}".format(server_agents[0].agent_id))

        domain = framework.getAgentProperty(server_agents[0].agent_id,'domain')
    else:
        print("HTTP server task already running.")
        domain = framework.getAgentProperty(agent,'domain')
    
    # Run the classify task
    unique_key = client + '-' + str(randint(0, 1000000))
    framework.runTask(client + ':' + "image classification",classify_agents[0],
                                docker_image='jnoor/classify:v1',
                                environment={'INPUT_URL':'https://' + domain + ':3003/' + unique_key + '-latest.jpg',
                                             'OUTPUT_URL':'https://' + domain + ':3003/' + client + '-predictions.jpg',
                                             'OUTPUTRESULT_URL':'https://' + domain + ':3003/' + client + '-results.json'})
    # Run the classify task

    framework.runTask(client + ':' + "camera task",cam_agents[0],
                                docker_image='jnoor/cameraalpine:v1',
                                environment={'SERVER_HOST':'https://' + domain + ':3003/' + unique_key + '-latest.jpg'})
    print("Started classify task on agent: {}".format(classify_agents[0].agent_id))
    print("Started camera task on agent: {}".format(cam_agents[0].agent_id))


if __name__ == '__main__':  # pragma: no cover
    parser = argparse.ArgumentParser(description='Launch a CoAP Resource Manager Framework')
    parser.add_argument('--host', required=True, help='the Edge RM Master IP to register with.')
    parser.add_argument('--port', required=False, default=80, help='the Edge RM Master port to register on.')
    parser.add_argument('--client', help='the client ID that submitted this task')
    parser.add_argument('--camera', help='the camera to use')
    args = parser.parse_args()
    main(args.host, args.port, args.client, args.camera)
