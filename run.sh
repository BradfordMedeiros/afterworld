#!/bin/bash

BASE_DIRECTORY=$(pwd)
SCRIPT_FOLDER=$BASE_DIRECTORY/scripts

BINARY="./build/modengine"
COMMAND_ARGS="\
  -x native/main \
  -a sqldir=../afterworld/data/sql/ \
  --data ../afterworld/data/temp/ \
  -a 'light=1 1 1' \
  -r ../afterworld/scenes/main.rawscene \
  --font ./res/fonts/Walby-Regular.ttf \
  $@"

#   --font ./res/fonts/ocr.ttf \

if [ "$1" == "gdb" ]; then
  (
    cd ../ModEngine && gdb "$BINARY" -ex "run  $COMMAND_ARGS"
  )
elif [ "$1" == "dry" ]; then
  echo "command: $COMMAND_ARGS"
  exit 1
else 
  (
    cd ../ModEngine && bash -c "$BINARY $COMMAND_ARGS"
  )
fi 


