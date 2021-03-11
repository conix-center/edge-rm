sleep 1
/opt/vc/bin/raspistill -o dockerimg.jpg
/usr/bin/curl --request POST --data-binary "@dockerimg.jpg" $SERVER_HOST