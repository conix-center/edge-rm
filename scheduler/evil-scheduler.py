#!/usr/bin/env python3
import sys
import argparse
import pydig

from scheduler_library import Framework

# I am a jerk.

# I will consume all of the resources you give me.

# Let me consume you.

def main(host, port):  # pragma: no cover

    #declare a framework
    framework = Framework("EvilMF", host, port)

    counter = 1
    while True:
        sleep(5)
        # Get an offer
        offers = framework.getOffers()

        print("Got offers.")
        for offer in offers:
            print("Agent", offer.agent_id)
            agent_id = offer.agent_id
            print("Resources:")
            resources = dict()
            for resource in offer.resources:
                resources[resource.name] = resource.scalar.value
            print(resources)
            for attribute in offer.attributes:
                print(attribute.name, ":", attribute.text.value)

            # Run a task consuming all available resources
            framework.runTask("DummyTask-" + str(counter),agent_id,
                                        resources,
                                        docker_image='jnoor/dummy:v1',
                                        docker_port_mappings={},
                                        environment={})
            counter += 1

if __name__ == '__main__':  # pragma: no cover
    parser = argparse.ArgumentParser(description='Launch the Evil Resource Manager Framework')
    parser.add_argument('--host', required=True, help='the Edge RM Master IP to register with.')
    parser.add_argument('--port', required=False, default=5683, help='the Edge RM Master port to register on.')
    args = parser.parse_args()
    main(args.host, args.port)
