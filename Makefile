# Assumes modengine directory is pulled into ../ModEngine

all: afterworld
release: afterworld_release

modengine:
	@echo "bringing in modengine"
	@(cd ../ModEngine && make modengine additional_src=../../afterworld/src)

modengine_release:
	@echo "bringing in modengine release"
	@(cd ../ModEngine && make modengine_release additional_src=../../afterworld/src)

afterworld: 
	@echo "making afterworld debug"
	#@(cd ./build && cmake -DCMAKE_BUILD_TYPE=Debug -DADDITIONAL_SRC=$(additional_src) .. && make all)

afterworld_release: 
	@echo "making afterworld release"
	#@(cd ./build && cmake -DCMAKE_BUILD_TYPE=Release -DADDITIONAL_SRC=$(additional_src) .. && make all)

clean:
	@echo "clean placeholder"

