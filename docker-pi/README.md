## Okay, I'm able to get access to an RPi camera from a Docker:

1. Create a Dockerfile such as the one in this directory. Make sure alpine apk installs the raspberrypi package.

2. Build the image: `docker build -t "hellotest:v1" .`

3. Enter the image: `docker run --entrypoint "/bin/sh" -it --device /dev/vchiq -v /opt/vc:/opt/vc --env LD_LIBRARY_PATH=/opt/vc/lib hellotest:v1`

4. From within, run raspistill: `/opt/vc/bin/raspistill -o test.jpg`

## References:

1. https://www.losant.com/blog/how-to-access-the-raspberry-pi-camera-in-docker
2. https://iotbytes.wordpress.com/create-your-first-docker-container-for-raspberry-pi-to-blink-an-led/