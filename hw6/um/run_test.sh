#!/bin/sh

./writetests >&2

for i in `ls *.um` 
do
        bn=`basename "$i" .um`
        input=/dev/null;
        output=/dev/null;
        if [ -e $bn.0 ]; then
                input=$bn.0
        fi
        if [ -e $bn.1 ]; then
                output=$bn.2
        fi

        ./um $bn.um >$output < $input
        if [ -e $bn.1 ]; then
                echo "-----------difference for $bn------------------" >&2
                diff $bn.2 $bn.1 >&2
        fi
done
