#!/bin/sh

export LDLIBRARYPATH=$LDLIBRARYPATH:/usr/local/lib
rm sprites
if [ $# -eq "0" ] 
then
    make sprites
else
    make all
fi
