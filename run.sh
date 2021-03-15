#!/bin/sh

BASE_DIRECTORY=$(pwd)
SCRIPT_FOLDER=$BASE_DIRECTORY/scripts

(
  cd ../mosttrusted/ModEngine && 
  ./build/modengine -x "$SCRIPT_FOLDER/level-select.scm" $@
)