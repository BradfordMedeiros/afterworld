#!/bin/bash

# This is needed so we can provide additional .so files to the steam runtime

export LD_LIBRARY_PATH="$(pwd)/runtime_libs:$LD_LIBRARY_PATH"

cd ./afterworld
./run.sh $@
