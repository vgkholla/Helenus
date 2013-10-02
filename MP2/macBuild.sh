#! /bin/bash

g++ -Iheaders/ -o $1 ./Main.cxx ./ConnectionHandler.cxx -lpthread -lboost_serialization-mt
