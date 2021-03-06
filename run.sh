#!/bin/sh

BASE_DIRECTORY=$(pwd)
SCENE_FOLDER=$BASE_DIRECTORY/scenes

(
  cd ../mosttrusted/ModEngine && 
  ./build/modengine -r "$SCENE_FOLDER/sandbox.rawscene" $@
)