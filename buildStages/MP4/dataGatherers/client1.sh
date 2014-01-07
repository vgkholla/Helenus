#! /bin/bash

if [ $# -ne 3 ]
then
	echo "Usage: $0 <name of client binary> <dest_ip> <dest_port>"
	exit 1
fi

CLIENT_BINARY=$1
DEST_ADDR=$2
DEST_PORT=$3
KEY="alok"
VALUE=0
./$CLIENT_BINARY --dst $DEST_ADDR --port $DEST_PORT --command insert\($KEY,$VALUE,1\)
for i in {1..100} 
do 
        if [ $VALUE -eq 10 ]; then
            VALUE=0
        fi
        VALUE=`expr $VALUE + 1`
        date=`date +'%d/%m/%Y_%H:%M:%S:%N'`
	echo "Inserting $KEY, $VALUE Date $date"
	./$CLIENT_BINARY --dst $DEST_ADDR --port $DEST_PORT --command update\($KEY,$VALUE,1\)
done

