#!/bin/bash
build() {
	(
		mkdir -p build-$1
		cd build-$1
		cmake -DCMAKE_BUILD_TYPE=$1 ..
		shift
		make "$@"
	)
}
if [ "$1" == "release" ]; then
	build "$@"
elif [ "$1" == "debug" ]; then
	build "$@"
elif [ "$1" == "both" ]; then
	shift
	build debug "$@" && build release "$@"
else
	build debug "$@"
fi
