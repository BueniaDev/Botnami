#!/bin/bash

if [ -z "$ASPATH" ]; then
    ASPATH=`which asl`
    ASPATH=${ASPATH%asl}
else
    ASPATH="$ASPATH"/
fi

for file in example/src/*.asm
do
	echo "Assembling file"
	${ASLPATH}asl -cpu 6809 ${file}

	if [ "$?" -ne 0 ]; then
	    echo "Could not assemble file"
	    exit 1
	fi

	pfile="${file%.*}.p"
	binfile="${file%.*}.bin"

	${ASLPATH}p2bin ${pfile} ${binfile}

	if [ "$?" -ne 0 ]; then
	    echo "Could not link file"
	    exit 1
	fi

	rm ${pfile}
done