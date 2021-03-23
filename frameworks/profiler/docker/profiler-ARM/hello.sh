sleep 10

echo "Starting..."

x=1
START=$(date +%s)
while [ $x -le 400000 ]
do
	x=$(($x + 1))
done
END=$(date +%s)

DIFF=$(($END-$START))

echo $DIFF

# HOST="conixdb.com"
# PORT=3001
# AGENT=4
# ENDPOINT="http://conixdb.com:3001/profile"

REQUEST_URL="${ENDPOINT}?id=${AGENT}&agent=${AGENT}&cpu=${DIFF}&ts=${TS}"
echo $REQUEST_URL

/usr/bin/curl $REQUEST_URL

wget $REQUEST_URL