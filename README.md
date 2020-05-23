Edge Resource Manager
=====================

Resource managers exist to collect computing resources from multiple disparate machines
and abstract their use into a unified interface for distributed applications. Existing
resources managers are focused on performing this task for cloud-based application.

This is a new resource manager that attempts to break that mold and extend
resource management across networks and computing classes. While the underlying
ideas of resource management are not new (and are heavily based on Apache Mesos), 
the implementation details are. The resource
manager agent is minimal such that it can be run on ultra-small computing devices. Diverse
networking protocols are supported so that one can aggregate resources across
bandwidth constrained networks and behind NATs. We intend to support executors
such as WASM in addition to traditional container environments so that multiple
tasks can be run in memory constrained environments.

Currently we are building a minimal testbed on which we can test these ideas
during development. The details of the testbed are outlined below.

### Server

We run the resource manager master on a linux server. It should be 
globally accessible. Current we have two resource managers under master/python.

Hopefully this will be combined into a single program at some point in the future.

### Gateway

Local linux-class devices serve as gateways for constrained network protocols
and perform local compute and data storage. Currently we are using raspberry
pis that are setup as thread border routers as described here: https://github.com/lab11/otbr.

To effectively use docker on an rpi you need to update the kernel to enable
cpu bandwidth controls as described here: https://github.com/hypriot/rpi-kernel

Then you should install docker as described here: https://phoenixnap.com/kb/docker-on-raspberry-pi

Finally install the requirements and run the agent in agent/python/websockets/agent

Hopefully in the future we will build this into an easy to deploy image...

You could also use your local computer as a gateway. For best performance it 
should have cgroup cpu bandwidth control enabled.

### Edge Devices

We are using permamotes as edge devices: github.com/lab11/permamote

Other edge devices could be used, but they should be able to act as COAP clients.

We hope to run WASM on these devices using the WASM Micro Runtime (WAMR): https://github.com/bytecodealliance/wasm-micro-runtime
