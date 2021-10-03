#!/bin/bash

BASE_DIRECTORY=$(pwd)
SCRIPT_FOLDER=$BASE_DIRECTORY/scripts

BINARY="./build/modengine"
COMMAND_ARGs="-x \"$SCRIPT_FOLDER/level-select.scm\" -j ../ModPlugins/build/sequencer.so -j ../ModPlugins/build/sql.so -a sqldir=../afterworld/data/ $@"

if [ "$1" == "gdb" ]; then
  (
    cd ../ModEngine && gdb "$BINARY" -ex "run  $COMMAND_ARGs"
  )
else 
  (
    cd ../ModEngine && bash -c "$BINARY $COMMAND_ARGs"
  )
fi 


