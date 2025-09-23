#!/bin/bash

export LD_LIBRARY_PATH="$(pwd)/build/runtime_libs:$LD_LIBRARY_PATH"


BASE_DIRECTORY=$(pwd)
SCRIPT_FOLDER=$BASE_DIRECTORY/scripts

BINARY="./build/modengine"
COMMAND_ARGS="\
  -x native/main \
  -a sqldir=../afterworld/data/sql/ \
  --data ../afterworld/data/temp/ \
  -r ../afterworld/scenes/main.rawscene \
  --font ../ModEngine/res/fonts/ocr.ttf \
  $@"

#   --font ./res/fonts/ocr.ttf \

if [ "$1" == "gdb" ]; then
  (
    cd ../ModEngine && gdb "$BINARY" -ex "run  $COMMAND_ARGS"
  )
elif [ "$1" == "dry" ]; then
  echo "command: $COMMAND_ARGS"
  exit 1
elif [ "$1" == "connect" ]; then
  (
     cd ../ModEngine && bash -c "$BINARY -a config-server=127.0.0.1:8085 $COMMAND_ARGS"
  )
else 
  (
    cd ../ModEngine && bash -c "$BINARY $COMMAND_ARGS"
  )
fi 


