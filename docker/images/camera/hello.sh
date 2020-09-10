/opt/vc/bin/raspistill -o dockerimg.jpg

/usr/bin/curl --request POST --data-binary "@dockerimg.jpg" http://172.17.40.64:3002/image
