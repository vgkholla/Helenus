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
    #key=`cut -f 1 -d ':' t`
    #echo $key
    ./$CLIENT_BINARY --dst $DEST_ADDR --port $DEST_PORT --command lookup\($key\)
done < /Users/Holla/Dropbox/Masters/MCS/Fall2013/CS425-DistributedSystems/ds-mps/MP4/indexes250.txt

