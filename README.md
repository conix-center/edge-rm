Edge Resource Manager
=====================

A new resource manager for heterogeneous devices that may exist throughout
a network of sensing and control. Should be able to run from servers all the way
down to Cortex-M class embedded devices and everywhere in between. 

The idea is to heavily model the initial implementation off of Apache Mesos, 
then add features as necessary to support our use cases.

The first goal is to be able to register agents running on a server, Rpi, and embedded node
then launch a task in a WASM runtime on those devices.
