The Zephyr agent allows one to run an agent on an embedded device. 
This means two things:

1) The agent uses low power embedded networking protocols (currently openthread)
2) The agent uses a low memory container executor (WASM/WAMR)

## Building the Agent for Zephyr

1) Follow the [Zephyr getting started guide](https://docs.zephyrproject.org/latest/getting_started/index.html) 
for your host OS. Make sure that you can successfully flash the blinky example to your platform.

2) Copy the custom `west.yml` file into the zephyr folder that is create. This adds the external
Paho project for mqttSN, and it changes the openthread directory to use a fork with a modified openthreead
library. Then update the dependencies

```shell
cp west.yml zephyr/west.yml
west update
```

3) Bootstrap the new version of openthread you have copied

```shell                                                                       
cd modules/lib/openthread                                                      
./script/bootstrap                                                             
./bootstrap                                                                    
``` 

4) Configure the agent

5) Build the agent
