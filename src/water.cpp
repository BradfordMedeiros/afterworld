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
		auto aabb = gameapi -> getModAABB(waterId);
		modassert(aabb.has_value(), "water does not have an aabb");
		auto topOfWater = aabb.value().max.y;
    auto waterObjAttr = gameapi -> getGameObjectAttr(waterId);
    auto waterDensityOpt = getFloatAttr(waterObjAttr, "water-density");
    auto waterDensity = waterDensityOpt.has_value() ? waterDensityOpt.value() : 10.f;
    auto waterGravityOpt = getFloatAttr(waterObjAttr, "water-gravity");
    auto waterGravity = waterGravityOpt.has_value() ? waterGravityOpt.value() : -9.81;
		auto fluidViscosityOpt = getFloatAttr(waterObjAttr, "water-viscosity");
		auto fluidViscosity = fluidViscosityOpt.has_value() ? fluidViscosityOpt.value() : 0.1f;

		std::vector<objid> idsToErase = {};
		for (auto submergedObjId : submergedObjects){
			if (!gameapi -> getGameObjNameForId(submergedObjId).has_value()){
				idsToErase.push_back(submergedObjId);
				continue;
			}
			auto submergedAttr = gameapi -> getGameObjectAttr(submergedObjId);
			
			// Get percentage of the object submerged by volume.  Uses AABS of water + submerged object, determines based upon if in object and y values only 
			auto submergedAabb = gameapi -> getModAABB(submergedObjId);
			float submergedObjWidth = submergedAabb.value().max.x - submergedAabb.value().min.x;
			float submergedObjHeight = submergedAabb.value().max.y - submergedAabb.value().min.y;
			float submergedObjDepth = submergedAabb.value().max.z - submergedAabb.value().min.z;
			auto submergedVolume = submergedObjWidth * submergedObjHeight * submergedObjDepth;
			auto unsubmergedTop =  submergedAabb.value().max.y - topOfWater;
			auto submergedHeight = submergedAabb.value().max.y - submergedAabb.value().min.y;
			auto submergedTop = submergedHeight - unsubmergedTop;
			auto percentage = std::min(1.f, submergedTop / submergedHeight);


			// Fluid Drag
			auto submergedObjVelocity = getVec3Attr(submergedAttr, "physics_velocity").value();
			//modlog("water", "submerged velocity: " + print(submergedObjVelocity));

			auto crossSectionalAreaYZ = submergedHeight * submergedObjDepth;
			auto fluidDragX = (submergedObjVelocity.x * submergedObjVelocity.x) * crossSectionalAreaYZ;
			if (submergedObjVelocity.x > 0){
				fluidDragX *= -1;
			}

			auto crossSectionalAreaXZ = submergedObjWidth * submergedObjDepth;
			auto fluidDragY = (submergedObjVelocity.y * submergedObjVelocity.y) * crossSectionalAreaXZ;
			if (submergedObjVelocity.y > 0){
				fluidDragY *= -1;
			}

			auto crossSectionalAreaXY = submergedObjWidth * submergedObjHeight;
			auto fluidDragZ = (submergedObjVelocity.z * submergedObjVelocity.z) * crossSectionalAreaXY;
			if (submergedObjVelocity.z > 0){
				fluidDragZ *= -1;
			}

			auto totalDrag = 0.5f * waterDensity * glm::vec3(0.f, fluidDragY, 0.f) * fluidViscosity;

			//modlog("water", "fluid drag: " + print(totalDrag));

			// calculate new forces based on percentage
			auto submergedGravity = getVec3Attr(submergedAttr, "physics_gravity").value().y;

			// Water gravity could be just the same as world gravity, but because i want to have tunable control this is separate 
			float upwardForce =  waterDensity * waterGravity * (submergedVolume * percentage /* volume of part of object submerged (based on AABB)*/);  
			auto waterEffectOnGravity = (waterGravity - submergedGravity) * percentage; // saying that the water has an effective gravity proportional to the amount of the object submerged

			auto forceVec = glm::vec3(0.f, upwardForce + waterEffectOnGravity, 0.f) + totalDrag;
			//modlog("water", "force vec is: " + print(forceVec) + ", gravity correction: " + std::to_string(waterEffectOnGravity) + ", buyoant force: " + std::to_string(upwardForce) + ", submerged percentage: " + std::to_string(percentage));
      gameapi -> applyForce(submergedObjId, forceVec);


			auto submergedObjAVelocity = getVec3Attr(submergedAttr, "physics_avelocity").value();
			auto areaCube =  std::cbrt(submergedObjWidth * submergedObjHeight * submergedObjDepth);
			auto crossSectionalArea =  areaCube * areaCube;
			auto fluidADragX = (submergedObjAVelocity.x * submergedObjAVelocity.x) * crossSectionalArea;
			if (submergedObjAVelocity.x > 0){
				fluidADragX *= -1;
			}
			auto fluidADragY = (submergedObjAVelocity.y * submergedObjAVelocity.y) * crossSectionalArea;
			if (submergedObjAVelocity.y > 0){
				fluidADragY *= -1;
			}
			auto fluidADragZ = (submergedObjAVelocity.z * submergedObjAVelocity.z) * crossSectionalArea;
			if (submergedObjAVelocity.z > 0){
				fluidADragZ *= -1;
			}

			auto totalADrag = 0.5f * waterDensity * glm::vec3(fluidADragX, fluidADragY, fluidADragZ) * fluidViscosity;
      gameapi -> applyTorque(submergedObjId, totalADrag);
		}
		for (auto id : idsToErase){
			submergedObjects.erase(id);
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
  	if (isPaused()){
  		return;
  	}
  	Water* water = static_cast<Water*>(data);
  	applyWaterForces(*water);
  };

	 return binding;
}