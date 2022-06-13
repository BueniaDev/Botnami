#!/bin/bash

if [[ ! -f "libbotnami.a" ]]; then
	echo "Run this script from the directory where you built the Botnami engine."
	exit 1
fi

# mkdir -p dist

# if [ -d "Vecnami" ]; then
# 	for lib in $(ldd Vecnami/vecnami.exe | grep mingw | sed "s/.*=> //" | sed "s/(.*)//"); do
# 		cp "${lib}" dist
# 	done
# 	cp Vecnami/vecnami.exe dist
# fi

