#!/bin/bash

if [[ ! -f "Botnami/libbotnami.a" ]]; then
	echo "Run this script from the directory where you built the Botnami engine."
	exit 1
fi

mkdir -p dist

if [ -d "example" ]; then
	for lib in $(ldd example/botnami-test.exe | grep mingw | sed "s/.*=> //" | sed "s/(.*)//"); do
		cp "${lib}" dist
	done
	cp example/botnami-test.exe dist
	cp ../example/src/*.bin dist
fi