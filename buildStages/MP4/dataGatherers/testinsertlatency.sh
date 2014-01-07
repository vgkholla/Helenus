#! /bin/bash

CLIENT_BINARY=$1
DEST_ADDR=$2
DEST_PORT=$3

for i in {1..1000} 
do 
	KEY=$RANDOM
	VALUE=$RANDOM
	echo "Inserting $KEY, $VALUE"
        time1=`date +'%d/%m/%Y_%H:%M:%S:%N' | cut -d ':' -f 4`
	./$CLIENT_BINARY --dst $DEST_ADDR --port $DEST_PORT --command insert\($KEY,$VALUE\)
        time2=`date +'%d/%m/%Y_%H:%M:%S:%N' | cut -d ':' -f 4`
        inserttt=`expr $time2 - $time1`
        insertms=`expr $inserttt / 1000000`
        echo $insertms >> file1
        time1=`date +'%d/%m/%Y_%H:%M:%S:%N' | cut -d ':' -f 4`
        ./$CLIENT_BINARY --dst $DEST_ADDR --port $DEST_PORT --command lookup\($KEY\)
        time2=`date +'%d/%m/%Y_%H:%M:%S:%N' | cut -d ':' -f 4`
        lookupt=`expr $time2 - $time1`
        lookupms=`expr $lookupt / 1000000`
        ./$CLIENT_BINARY --dst $DEST_ADDR --port $DEST_PORT --command delete\($KEY\)
        echo $lookupms >> file2
done
