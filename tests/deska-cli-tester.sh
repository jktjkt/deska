#!/bin/bash

DESKA_CFGGEN_BACKEND=fake ./deska-cli -e ${1} > /dev/null 2>&1
DESKA_CFGGEN_BACKEND=fake ./deska-cli -d ${2}_result > /dev/null 2>&1

if [[ `diff ${2} ${2}_result` ]]; then
    echo "Processing file ${1} failed. Dumps ${2} and ${2}_result differ."
    exit 10
fi

echo "Processing file ${1} OK"
