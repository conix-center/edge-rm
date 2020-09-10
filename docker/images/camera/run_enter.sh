docker run --entrypoint "/bin/sh" -it --device /dev/vchiq -v /opt/vc:/opt/vc:ro -v /lib:/lib:ro -v /usr/lib:/usr/lib:ro -v /opt/vc/lib:/opt/vc/lib:ro --env LD_LIBRARY_PATH=/opt/vc/lib hellocamera:v1
