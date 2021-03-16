sleep 1

x=1
START=$(date +%s)
while [ $x -le 1000000 ]
do
	x=$(($x + 1))
done
END=$(date +%s)

DIFF=$(($END-$START))

echo $DIFF

HOST="https://conixdb.com"
PORT=3001

REQUEST_URL="${HOST}?time=${DIFF}"

echo "${HOST}:${PORT}?id=${AGENTID}&agent=${AGENTID}&time=${DIFF}"
REQUEST_URL="${HOST}:${PORT}?id=${AGENTID}&agent=${AGENTID}&time=${DIFF}"

/usr/bin/curl $REQUEST_URL