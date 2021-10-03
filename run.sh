#!/bin/bash

BASE_DIRECTORY=$(pwd)
SCRIPT_FOLDER=$BASE_DIRECTORY/scripts

if [ "$1" == "gdb" ]; then
  (
    cd ../ModEngine && 
    gdb ./build/modengine -ex \
      "run  \
      -x \"$SCRIPT_FOLDER/level-select.scm\" \
      -j ../ModPlugins/build/sequencer.so \
      -j ../ModPlugins/build/sql.so \
      -a sqldir=../afterworld/data/ \
      $@"
  )
else 
  (
    cd ../ModEngine && 
    ./build/modengine \
      -x "$SCRIPT_FOLDER/level-select.scm" \
      -j ../ModPlugins/build/sequencer.so \
      -j ../ModPlugins/build/sql.so \
      -a sqldir=../afterworld/data/ \
      $@
  )
fi 


