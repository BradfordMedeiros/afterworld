# Assumes modengine directory is pulled into ../ModEngine

all: afterworld
release: afterworld_release

afterworld:
	@(cd ../ModEngine && make modengine additional_src=../../afterworld/src)

afterworld_release: 
	@(cd ../ModEngine && make modengine_release additional_src=../../afterworld/src)

validate: 
	@(cd ../ModEngine && ./build/modengine --validate ../afterworld/scenes --validate ../ModEngine/res/)

package: afterworld_release
	@(./package.sh)
	
clean:
	@echo "clean placeholder"


