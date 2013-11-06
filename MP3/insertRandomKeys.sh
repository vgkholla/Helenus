#! /bin/bash

if [ $# -ne 3 ]
then
	echo "Usage: $0 <name of client binary> <dest_ip> <dest_port>"
	exit 1
fi

CLIENT_BINARY=$1
DEST_ADDR=$2
DEST_PORT=$3

for i in {1..1000} 
do 
	KEY=$RANDOM
	VALUE=$RANDOM
	echo "Inserting $KEY, $VALUE"
	./$CLIENT_BINARY --dst $DEST_ADDR --port $DEST_PORT --command insert\($KEY,$VALUE\)
done