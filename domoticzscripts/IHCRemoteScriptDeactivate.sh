#!/bin/bash
directory=$(dirname "$0")
script=$directory/IHCRemoteScript.sh
#curl -d "{\"type\":\"deactivateOutput\",\"moduleNumber\":$1,\"ioNumber\":$2,\"id\":0}" -H "Content-Type: application/jso" -X POST http://192.168.1.72:8081/ihcrequest
$script deactivateOutput $1 $2
$script setOutput $1 $2 false
