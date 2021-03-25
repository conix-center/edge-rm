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

# AGENT=1248
# ENDPOINT="http://conixdb.com:3001/profile"
# DOMAINS="conixdb.com"

# DOMAIN0="conixdb.com"
# DOMAIN1="conixdb.com"

TIMES=""

DOMAININDEX=0
while :
do
	echo $DOMAININDEX
	IDXNAME="DOMAIN${DOMAININDEX}"
	ENTRY=$(eval echo \$$IDXNAME)
	echo $ENTRY
	if [ -z "${ENTRY}" ]
	then
		echo "empty"
		break
	else
		echo "not empty"
	fi
	time -o output.txt -f "%e" nc -zw5 "${ENTRY}" 80
	ELAPSED=$(cat output.txt)
	TIMES="${TIMES}+${ELAPSED}"
	DOMAININDEX=$((DOMAININDEX+1))
done

time -f "%e" nc -zw5 conixdb.com 80

REQUEST_URL="${ENDPOINT}?id=${AGENT}&agent=${AGENT}&cpu=${DIFF}&ts=${TS}&pings=${TIMES}"
echo $REQUEST_URL

/usr/bin/curl $REQUEST_URL

wget $REQUEST_URL