cd darknet
touch output.jpg

while true
do
	sleep 5
	wget -O latest.jpg $INPUT_URL
	if ! cmp output.jpg latest.jpg
	then
		./darknet detect cfg/yolov3.cfg yolov3.weights ./latest.jpg
		/usr/bin/curl --request POST --data-binary "@predictions.jpg" $OUTPUT_URL
		cp latest.jpg output.jpg
	fi
done

cd darknet
./darknet detect cfg/yolov3.cfg yolov3.weights ../latest.jpg