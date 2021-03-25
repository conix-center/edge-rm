#!/usr/bin/env python3
import sys
import argparse
import pydig
import time

from edgerm.framework import Framework

def main(host, port):  # pragma: no cover

    #declare a framework
    framework = Framework("Profiler Framework", host, port)

    # First we need to launch the server task if it's not already running
    agent = framework.getAgentInfoForRunningTask('ProfilerServer')

    #Get offers
    offers = framework.getOffers()

    # find all domains
    allDomains = []
    for agentWithDomain in offers:
        for att in agentWithDomain.attributes:
            if att.name == 'domain':
                allDomains.append(att.text.value)
    print(allDomains)

    domain = None
    if agent is None:
        #launch the task
        server_agents = framework.findAgents(offers, {'domain':None,'cpus':0.5,'mem':100000000})

        if len(server_agents) == 0:
            print("No available server agents.")
            return


        framework.runTask("ProfilerServer",server_agents[0],
                                docker_image='jnoor/profilerserver:v1',
                                docker_port_mappings={3001:3001},
                                environment={'SERVER_PORT':'3001'})

        print("Started server task on agent: {}".format(server_agents[0].agent_id))

        domain = framework.getAgentProperty(server_agents[0].agent_id,'domain')

        print ("Sleeping to allow server to come online...")
        time.sleep(10)
    else:
        print("Sensor server task already running.")
        domain = framework.getAgentProperty(agent,'domain')

    # okay now do a dns lookup for this domain
    print("Lookup: " + domain)
    ip = pydig.query(domain,'A')[0]

    #construct the profile task environment
    env = {}
    env['HOST'] = domain
    env['PORT'] = 3001
    env['TS'] = int(time.time())
    env['ENDPOINT'] = "http://" + str(ip) + ":3001/profile"

    for i in range(len(allDomains)):
        env['DOMAIN' + str(i)] = allDomains[i]

    agents = framework.findAgents(offers, {'cpus':0.5,"mem":10000000})
    for agent in agents:
        imageToUse = None
        for attribute in agent.attributes:
            if attribute.name == 'OS':
                # print(agent.agent_id, attribute.name, attribute.text.value)    
                if "x86" in attribute.text.value:
                    imageToUse = 'jnoor/profiler-x86:v1'
                elif 'arm' in attribute.text.value:
                    imageToUse = 'jnoor/profiler-arm:v1'

        if not imageToUse:
            print("No profiler image for ")
            print(agent)
            continue
        # if agent.agent_id != '14038003386728':
        #     continue
        env['AGENT'] = agent.agent_id
        print("Starting profiler task on agent: {}".format(agent.agent_id))
        framework.runTask("profiler-" + str(agent.agent_id), agent, docker_image=imageToUse, environment=env)

    # #Find a wasm agent
    # wasm_agents = framework.findAgents({'executors':'WASM','cpus':1.0,sensor_name:None})
    # if(len(wasm_agents) == 0):
    #     print("No available sensor agents.",file=sys.stderr)
    #     return

    # # Run the WASM task
    # wasm_file = open('../wasm/wasm-send/out.wasm','rb')
    # print("Running task with environment:")
    # print(env)
    # framework.runTask(args.client + ':' + "SensorSample",wasm_agents[0],wasm_binary=wasm_file.read(),environment=env)
    # print("Started sensor task on agent: {}".format(wasm_agents[0].agent_id))



if __name__ == '__main__':  # pragma: no cover
    parser = argparse.ArgumentParser(description='Launch the Profiler Meta-Framework')
    parser.add_argument('--host', required=True, help='the Edge RM Master IP to register with.')
    parser.add_argument('--port', required=False, default=80, help='the Edge RM Master port to register on.')
    args = parser.parse_args()
    main(args.host, args.port)
