#!/bin/bash

CONFIGURATION=$1

mkdir -p ../../solutions/solution_unix/$CONFIGURATION
pushd ../../solutions/solution_unix/$CONFIGURATION
cmake -G "Unix Makefiles" -S "../../../cmake/unix" -DCMAKE_BUILD_TYPE:STRING=$CONFIGURATION
cmake --build ./ --config $CONFIGURATION
popd