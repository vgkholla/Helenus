#! /bin/bash

CLIENT_BINARY=$1
DEST_ADDR=$2
DEST_PORT=$3
DEST_ADDR1=$4
DEST_ADDR2=$5
DEST_ADDR3=$6

for i in {1..1000} 
do 
	KEY=$RANDOM
	VALUE=$RANDOM
	echo "Inserting $KEY, $VALUE"
	./$CLIENT_BINARY --dst $DEST_ADDR --port $DEST_PORT --command insert\($KEY,$VALUE\)
done

./$CLIENT_BINARY --dst $DEST_ADDR --port $DEST_PORT --command show | grep '[0-9]:[0-9]' | wc -l >> keycount
./$CLIENT_BINARY --dst $DEST_ADDR1 --port $DEST_PORT --command show | grep '[0-9]:[0-9]' | wc -l >> keycount
./$CLIENT_BINARY --dst $DEST_ADDR2 --port $DEST_PORT --command show | grep '[0-9]:[0-9]' | wc -l >> keycount
./$CLIENT_BINARY --dst $DEST_ADDR3 --port $DEST_PORT --command show | grep '[0-9]:[0-9]' | wc -l >> keycount

