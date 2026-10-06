#!/bin/bash

type=$1

if [ -z "$type" ]
then
	type=Release
else
	type=$type
fi

cmake -B build -D CMAKE_BUILD_TYPE=$type -D BUILD_TESTS=OFF
cmake --build build --parallel
