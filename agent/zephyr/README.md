The Zephyr agent allows one to run an agent on an embedded device. 
This means two things:

This is not working yet!!

1) The agent uses low power embedded networking protocols (currently openthread)
2) The agent uses a low memory container executor (WASM/WAMR)

## Building the Agent for Zephyr

1) Install [nrfjprog](https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Command-Line-Tools/Download#infotabs)

2) You are going to _mostly_ Follow the [Zephyr getting started guide](https://docs.zephyrproject.org/latest/getting_started/index.html) 
for your host OS. When you get to the west init step, instead of doing what it says instead run

```shell
west init -m https://github.com/oubotong/zephyr-v2.3.99
cp west.yml zephyr/west.yml
west update
```

Copying the custom `west.yml` file into the zephyr folder that is created adds the external
Paho project for mqttSN, and it changes the openthread directory to use a fork with a modified openthreead
library. Then update the dependencies

Note that your zephyr root directory is now this directory instead of ~/zephyrproject. Any time
moving forward the guide says "~/zephyrproject" instead use ./ from _this_ directory.

3) Finish the getting started guide and make sure the blinky app runs

4) Configure the agent

TODO

5) Build the agent

```shell
west build -b nrf52840dk_nrf52840 app -p always -- -DCONF_FILE="prf_nrf52840_dk.conf  overlay-ot.conf" -DWAMR_BUILD_TARGET=THUMB -DWAMR_BUILD_AOT=0
west flash
```
