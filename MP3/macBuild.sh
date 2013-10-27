#! /bin/bash

if [ $# -ne 1 ]
then
	echo "Usage: $0 <name of binary>"
	exit 1
fi

g++ -Iheaders/ -o $1 ./Main.cxx ./ConnectionHandler.cxx -lpthread -lboost_system-mt -lboost_serialization-mt
