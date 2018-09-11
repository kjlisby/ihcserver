#!/bin/bash
directory=$(dirname "$0")
script=$directory/IHCRemoteScript.sh
$script activateInput $1 $2
