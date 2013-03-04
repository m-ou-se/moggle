#!/bin/bash
if [ "$1" == "release" ]; then
	shift
	make -C build-release "$@"
elif [ "$1" == "both" ]; then
	shift
	make -C build-debug "$@" && make -C build-release "$@"
else
	make -C build-debug "$@"
fi
