#!/bin/bash
if [ $# -lt 1 ]
then
	echo "Usage: $0 elfFile"
	exit 1
fi

entryPoint=$(readelf -h $1 | grep "Entry point address" | awk '{print $4}')
objdump -j .text -d $1 | ./ungcc $entryPoint
