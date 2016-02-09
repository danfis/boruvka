#!/bin/bash

i=0
while [ $i != 100 ]; do
    ./msg-schema-gen s1 >s1.txt
    ./msg-schema-print s1 >s2.txt
    if ! diff s1.txt s2.txt; then
        exit -1
    fi
    i=$(($i + 1))
done
rm s1 s1.txt s2.txt
