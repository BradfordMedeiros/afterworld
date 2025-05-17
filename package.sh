#!/bin/bash

set -e 

(
	cd ../gameresources/ && make -j12 all
)

mkdir -p ./build
mkdir -p ./build/afterworld
mkdir -p ./build/ModEngine/build

cp ../ModEngine/build/modengine ./build/ModEngine/build
cp -r ../ModEngine/build/runtime_libs/ ./build
cp -r ./data ./build/afterworld
cp ./run.sh ./build/afterworld

(cd ../ModEngine && ./build/modengine --package ../afterworld/build/game.mod --pak ./res --pak ./res/ --pak ./docs/ --pak ../gameresources/build --pak ../afterworld/scenes/ --pak ../afterworld/design/ --pak ../gameresources/sound/ --pak ../gameresources/video --pak ../gameresources/textures/  --pak ../gameresources/ui/ --pak ../afterworld/shaders/)
strip ./build/ModEngine/build/modengine
cp ./build-run.sh ./build/run.sh

