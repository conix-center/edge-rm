cd darknet

# INPUT_URL="http://localhost:3000/test"

while true
do
	sleep 1
	does_file_exist=$(curl -o /dev/null --silent --head --write-out '%{http_code}' $INPUT_URL)
	should_be_200=$(echo "200")
	echo "Checking..."
	if [ "$does_file_exist" = "$should_be_200" ]; then
		break
	fi
done
echo "it exists!"

sleep 2
wget -O latest.jpg $INPUT_URL
./darknet detector test ./cfg/coco.data yolov4-tiny.cfg yolov4-tiny.weights -thresh 0.1 -out results.json ./latest.jpg
/usr/bin/curl --request POST --data-binary "@predictions.jpg" $OUTPUT_URL
/usr/bin/curl --request POST --data-binary "@results.json" $OUTPUTRESULT_URL