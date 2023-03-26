#include "./water.h"

extern CustomApiBindings* gameapi;


struct Water {
	std::map<objid, std::set<objid>> objectsInWater;
};


// needs improvement taking into account masses, etc, but about the right idea. 
// get bounding box 
// rotation of bounding box 
// get position of water center
// calculate how far down in the water the box is 
// just say the boyant force is how far the box is down * water density 
void applyWaterForces(Water& water){
	for (auto &[waterId, submergedObjects] : water.objectsInWater){
		auto boundingHeight = 30.f;
		auto halfBoundingHeight = 0.5 * boundingHeight;
		auto waterPos = gameapi -> getGameObjectPos(waterId, true);

		auto aabb = gameapi -> getModAABB(waterId);
		modassert(aabb.has_value(), "water does not have an aabb");
		auto topOfWater = aabb.value().max.y;
    auto waterObjAttr = gameapi -> getGameObjectAttr(waterId);
    auto waterDensity = getFloatAttr(waterObjAttr, "water-density").value();

		for (auto submergedObjId : submergedObjects){
			auto submergedObjectPos = gameapi -> getGameObjectPos(submergedObjId, true);

			auto submergedAabb = gameapi -> getModAABB(submergedObjId);
			auto submergedVolume = (submergedAabb.value().max.x - submergedAabb.value().min.x) * (submergedAabb.value().max.y - submergedAabb.value().min.y) * (submergedAabb.value().max.z - submergedAabb.value().min.z);
			auto unsubmergedTop =  submergedAabb.value().max.y - topOfWater;
			auto submergedHeight = submergedAabb.value().max.y - submergedAabb.value().min.y;
			auto submergedTop = submergedHeight - unsubmergedTop;
			auto percentage = std::min(1.f, submergedTop / submergedHeight);

			std::cout << "submerged percentage is: " << percentage << std::endl;

			float upwardForce = waterDensity * submergedVolume * percentage;  
			auto forceVec = glm::vec3(0.f, upwardForce, 0.f);
      gameapi -> applyForce(submergedObjId, forceVec);
		}
	}
}

void printWaterElements(Water& water){
	std::cout << "water: [";
	for (auto &[waterId, allObjects] : water.objectsInWater){
		std::cout << "( " << waterId << ", " << "[ ";
		for (auto id : allObjects){
			std::cout << id << " ";
		}
		std::cout << " ]" << ")" << std::endl;
	}
	std::cout << " ]" << std::endl;
}

CScriptBinding waterBinding(CustomApiBindings& api, const char* name){
	auto binding = createCScriptBinding(name, api);
	binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Water* water = new Water;
    water -> objectsInWater = {};
    return water;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Water* water = static_cast<Water*>(data);
    delete water;
  };

  binding.onCollisionEnter = [](objid id, void* data, int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal) -> void {
    Water* water = static_cast<Water*>(data);
    auto obj1Attr = gameapi -> getGameObjectAttr(obj1);
    auto obj1IsWater = getStrAttr(obj1Attr, "water").has_value();
    auto obj2Attr = gameapi -> getGameObjectAttr(obj2);
    auto obj2IsWater = getStrAttr(obj2Attr, "water").has_value();
    modlog("water", "obj1 water: " + print(obj1IsWater) + ", obj2 water: " + print(obj2IsWater));
    if (obj1IsWater){
    	if (water -> objectsInWater.find(obj1) == water -> objectsInWater.end()){
    		water -> objectsInWater[obj1] = {};
    	}
    	water -> objectsInWater.at(obj1).insert(obj2);
    }else if (obj2IsWater){
    	if (water -> objectsInWater.find(obj2) == water -> objectsInWater.end()){
       	water -> objectsInWater[obj2] = {};
    	}
    	water -> objectsInWater.at(obj2).insert(obj1);
    }
    printWaterElements(*water);
  };
  binding.onCollisionExit = [](objid id, void* data, int32_t obj1, int32_t obj2) -> void {
    Water* water = static_cast<Water*>(data);
    if (water -> objectsInWater.find(obj1) != water -> objectsInWater.end()){
    	water -> objectsInWater.at(obj1).erase(obj2);
    }
    if (water -> objectsInWater.find(obj2) != water -> objectsInWater.end()){
    	water -> objectsInWater.at(obj2).erase(obj1);
    }
    printWaterElements(*water);
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
  	Water* water = static_cast<Water*>(data);
  	applyWaterForces(*water);
  };

	 return binding;
}