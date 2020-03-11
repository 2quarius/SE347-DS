#!/bin/bash
make clean

for dir in `ls --file-type -1`;
do
    if [`echo $dir | grep "/$"`]; then
        dir=`basename $dir`;
        rm -rf $dir;
    fi
done