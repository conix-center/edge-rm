EdgeRM Master
============

To run the EdgeRM Master

1) Clone this report recursively or

```
$ git submodule update --init
```

2) Install docker on your host system (currently support linux and mac)

3) Install requirements

```
$ pip3 install -r requirements.txt
```

4) Start the master. We recommend having ports 5683 and 80 open, although
They can be changed with options 'port' and 'api-port'

```
$ ./master.py --host <your_host_ip>
```
