#!/bin/bash
cpids=$1

while [ -n "$cpids" ]
do
    last_pid=$cpids
    #echo "child of $cpids is"
    cpids=`pgrep -P $cpids`
done
#echo "last pid is $last_pid"
#kill $last_pid
#echo "the son of $1 is $spids, gonna kill it"
kill $last_pid
echo "terminated process beacuse of timeout"
