#!/bin/bash
directory=$(dirname "$0")
script=$directory/IHCRemoteScript.sh
$script deactivateInput $1 $2
