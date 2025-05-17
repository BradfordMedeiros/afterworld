#!/bin/bash

# This is needed so we can provide additional .so files to the steam runtime

export LD_LIBRARY_PATH="$(pwd)/lib:$LD_LIBRARY_PATH"

cd ./afterworld
./run.sh $@
