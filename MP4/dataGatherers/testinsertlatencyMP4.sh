#! /bin/bash

CLIENT_BINARY=$1
DEST_ADDR=$2
DEST_PORT=$3
CONSISTENCY=$4

for i in {1..1000} 
do 
	KEY=$RANDOM
	VALUE=$RANDOM
        array[$i]=$KEY
	echo "Inserting $KEY, $VALUE"
	./$CLIENT_BINARY --dst $DEST_ADDR --port $DEST_PORT --command $CONSISTENCY-insert\($KEY,$VALUE\)
done
for i in {1..100}
do
        VALUE=$RANDOM
        echo ${array[$i]}
        time1=`date +'%d/%m/%Y_%H:%M:%S:%N' | cut -d ':' -f 4`
        ./$CLIENT_BINARY --dst $DEST_ADDR --port $DEST_PORT --command $CONSISTENCY-update\(${array[$i]},$VALUE\)
        time2=`date +'%d/%m/%Y_%H:%M:%S:%N' | cut -d ':' -f 4`
        updatet=`expr $time2 - $time1`
        updatems=`expr $updatet / 1000000`
        echo $updatems >> file1
done
