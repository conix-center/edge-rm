### Gateway

Local linux-class devices serve as gateways for constrained network protocols
and perform local compute and data storage. Currently we are using raspberry
pis that are setup as thread border routers.

You need a custom kernel to run docker with all the cgroup features on a pi ([see this](github.com/hypriot/rpi-kernel)). We provide an 
image with this kernel that is running thread border router software [here](https://drive.google.com/drive/u/1/folders/1SPO9n25aIeH7cvcsD7acbq7WBMO16mKg).
Setup for this image, which is based off the lab11 open thread border router image, is described [here](https://github.com/lab11/otbr).

The image comes with a copy of this repo that automatically performs a git pull, updates library
dependencies, and starts a resource manager agent on boot.

### Edge Devices

We are using permamotes as edge devices: github.com/lab11/permamote

Other edge devices could be used, but they should be able to act as COAP clients.

We hope to run WASM on these devices using the WASM Micro Runtime (WAMR): https://github.com/bytecodealliance/wasm-micro-runtime
