#!/bin/sh

rm sprites
export LDLIBRARYPATH=$LDLIBRARYPATH:/usr/local/lib

if [ $# -eq "0" ] 
then
    make debug
else
    make all
fi
