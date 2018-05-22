#!/bin/bash
cpids=$1

#while [ -n "$cpids" ]
#do
    #echo "child of $cpids is"
    spids=`pgrep -P $cpids`
    #echo "$cpids"
#done
#echo "last pid is $last_pid"
#kill $last_pid
#echo "the son of $1 is $spids, gonna kill it"
kill $spids
echo "terminated process beacuse of timeout"
