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

package-debug: afterworld 
	@(./package.sh)

arcade: ./arcade/Vagrantfile 
	@(mkdir -p ./build/vagrant)
	@(cp ./arcade/Vagrantfile ./build/vagrant/)
	@(cp ./arcade/setup.sh ./build/vagrant/)
	@(cd ./build/vagrant && vagrant up)

upload-arcade:
	@(cd ./build/vagrant && vagrant ssh -c "sudo rm -rf /arcade/*")
	@(cd ./build/vagrant && vagrant upload ../../.. /arcade/)

clean-arcade:
	@(cd ./build/vagrant && vagrant destroy -f)
	@(rm -rf ./build/vagrant/)


clean:
	@echo "clean placeholder"


