# Assumes modengine directory is pulled into ../ModEngine

all: afterworld
release: afterworld_release

modengine:
	@(cd ../ModEngine && make modengine additional_src=../../afterworld/src)

modengine_release: 
	@(cd ../ModEngine && make modengine_release additional_src=../../afterworld/src)

afterworld: modengine

afterworld_release: modengine_release 

validate: 
	@(cd ../ModEngine && ./build/modengine --validate ../)


clean:
	@echo "clean placeholder"

