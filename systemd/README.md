To get the edge rm running on your linux machine you can use this service file

1) checkout edge-rm _recursively_ to your home directory..
```
git clone --recursive https://github.com/conix-center/edge-rm
```

2) Fix up any paths in edge-rm-agent.service to be correct for your system

3) copy service file to /etc/systemd/system/.
```
$ sudo cp edge-rm-agent.service /etc/systemd/system/.
```

4) Start the service
```
$ sudo systemctl enable edge-rm-agent
$ sudo systemctl start edge-rm-agent
```
