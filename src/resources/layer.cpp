#include "./layer.h"

PhysicsLayer physicsLayers;

int bonesAndObjects(){
	return 0b1101;
}

void printLayerInfo(){
	std::cout << "layer info : main X bones = " << ((physicsLayers.main & physicsLayers.bones) == 0) << std::endl;
	std::cout << "layer info : main X enemyBounding = " << ((physicsLayers.main & physicsLayers.enemyBounding) == 0) << std::endl;
	std::cout << "layer info : main X particles = " << ((physicsLayers.main & physicsLayers.particles) == 0) << std::endl;
	std::cout << "layer info : main X effects = " << ((physicsLayers.main & physicsLayers.effects) == 0) << std::endl;

	std::cout << "layer info : bones X main = " << ((physicsLayers.bones & physicsLayers.main) == 0) << std::endl;
	std::cout << "layer info : bones X enemyBounding = " << ((physicsLayers.bones & physicsLayers.enemyBounding) == 0) << std::endl;
	std::cout << "layer info : bones X particles = " << ((physicsLayers.bones & physicsLayers.particles) == 0) << std::endl;
	std::cout << "layer info : bones X effects = " << ((physicsLayers.bones & physicsLayers.effects) == 0) << std::endl;

	std::cout << "layer info : enemyBounding X main = " << ((physicsLayers.enemyBounding & physicsLayers.main) == 0) << std::endl;
	std::cout << "layer info : enemyBounding X bones = " << ((physicsLayers.enemyBounding & physicsLayers.bones) == 0) << std::endl;
	std::cout << "layer info : enemyBounding X particles = " << ((physicsLayers.enemyBounding & physicsLayers.particles) == 0) << std::endl;
	std::cout << "layer info : enemyBounding X effects = " << ((physicsLayers.enemyBounding & physicsLayers.effects) == 0) << std::endl;

	std::cout << "layer info : particles X main = " << ((physicsLayers.particles & physicsLayers.main) == 0) << std::endl;
	std::cout << "layer info : particles X bones = " << ((physicsLayers.particles & physicsLayers.bones) == 0) << std::endl;
	std::cout << "layer info : particles X enemyBounding = " << ((physicsLayers.particles & physicsLayers.enemyBounding) == 0) << std::endl;
	std::cout << "layer info : particles X effects = " << ((physicsLayers.particles & physicsLayers.effects) == 0) << std::endl;

	std::cout << "layer info : effects X main = " << ((physicsLayers.effects & physicsLayers.main) == 0) << std::endl;
	std::cout << "layer info : effects X bones = " << ((physicsLayers.effects & physicsLayers.bones) == 0) << std::endl;
	std::cout << "layer info : effects X enemyBounding = " << ((physicsLayers.effects & physicsLayers.enemyBounding) == 0) << std::endl;
	std::cout << "layer info : effects X particles = " << ((physicsLayers.effects & physicsLayers.particles) == 0) << std::endl;



	std::cout << "layer info : main = " << physicsLayers.main << std::endl;
	std::cout << "layer info : bones = " << physicsLayers.bones << std::endl;
	std::cout << "layer info : enemyBounding = " << physicsLayers.enemyBounding << std::endl;
	std::cout << "layer info : particles = " << physicsLayers.particles << std::endl;
	std::cout << "layer info : effects = " << physicsLayers.effects << std::endl;

}