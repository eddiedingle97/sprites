#!/bin/sh

rm sprites
export LDLIBRARYPATH=$LDLIBRARYPATH:/usr/local/lib

if [ $# -eq "0" ] 
then
    make sprites
else
    make all
fi
