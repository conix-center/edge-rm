cd darknet
touch output.jpg

while true
do
	sleep 2
	wget -O latest.jpg $INPUT_URL
	if ! cmp output.jpg latest.jpg
	then
		# ./darknet detect cfg/yolov3.cfg yolov3.weights ./latest.jpg
		./darknet detector test ./cfg/coco.data yolov4-tiny.cfg yolov4-tiny.weights -thresh 0.1 -out results.json ./latest.jpg
		/usr/bin/curl --request POST --data-binary "@predictions.jpg" $OUTPUT_URL
		/usr/bin/curl --request POST --data-binary "@results.json" $OUTPUTRESULT_URL
		cp latest.jpg output.jpg
	else
		echo "Same image..."
	fi
done