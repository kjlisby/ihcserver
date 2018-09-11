#!/bin/bash
directory=$(dirname "$0")
log=$directory/IHC.log
if [ $# -eq 4 ]; then
    cmd="curl -d {\"type\":\"$1\",\"moduleNumber\":$2,\"ioNumber\":$3,\"state\":$4,\"id\":0} -X POST http://DOMOTICZ-SERVER/ihcrequest"
else
    cmd="curl -d {\"type\":\"$1\",\"moduleNumber\":$2,\"ioNumber\":$3,\"id\":0} -X POST http://DOMOTICZ-SERVER/ihcrequest"
fi
for i in `seq 1 10`;
do
    $cmd
    if [ $? -eq 0 ] 
    then
        echo "`date`  $cmd - Success $i" >> $log
        exit 0
    fi
    sleep 3
done
echo "`date`  $cmd - Failure" >> $log
exit 1
