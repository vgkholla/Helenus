#!/bin/bash

CLIENT_BINARY=$1
DEST_ADDR=$2
DEST_PORT=$3
count=0
while read line
do
    #count=$((count+1))
    #if [ $count -eq 100 ];then
    #   break
    #fi
    key=`echo $line | cut -f 1 -d ':'`
    value=`echo $line | cut -f 3 -d ':'`
    if [ "$value" == "index" ]; then
        value=`echo $line | cut -f 4 -d ':'`
    fi
    #key=`cut -f 1 -d ':' t`
    echo $key
    echo $value
    ./$CLIENT_BINARY --dst $DEST_ADDR --port $DEST_PORT --command insert\($key,$value\)
done < /home/hollava2/Documents/Fall2013/DS/ds-mps/MP4/indexes1000.txt

