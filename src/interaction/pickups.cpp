#include "./pickups.h"

extern std::unordered_map<objid, Gem> gems;
extern std::unordered_map<objid, Breakable> breakables;

bool isControlledVehicle(int vehicleId);
bool isControlledPlayer(int playerIndex);

void updateGemStyle(objid id){
	auto& gem = gems.at(id);

	if (gem.alreadyPickedUp){
		setGameObjectMeshEnabled(id, false);
	}else if(!gem.canPickup){
		setGameObjectMeshEnabled(id, true);
		setGameObjectTint(id, glm::vec4(1.f, 1.f, 1.f, 0.2f));
	}else{
		setGameObjectMeshEnabled(id, true);
		setGameObjectTint(id, glm::vec4(1.f, 1.f, 1.f, 1.f));
	}
}

void addGem(objid id){
	auto gem = getSingleAttr(id, "gem");
	auto gemName = gem.value();
	bool alreadyPickedUp = hasCrystal(gemName);
	gems[id] = Gem{
		.name = gemName,
		.alreadyPickedUp = alreadyPickedUp,
	};
	updateGemStyle(id);
}

void doPickupGem(objid id){
	auto& gem = gems.at(id);
	if (gem.alreadyPickedUp){
		return;
	}
	if (!gem.canPickup){
		return;
	}

    gem.alreadyPickedUp = true;
    stageCrystal(gem.name);
    updateGemStyle(id);
}

bool isGem(objid id){
	return gems.find(id) != gems.end();
}

void handleGemCollision(int32_t obj1, int32_t obj2){
  if (isControlledVehicle(obj1)){
  	if (isGem(obj2)){
  		doPickupGem(obj2);
  	}
  }else if (isControlledVehicle(obj2)){
  	if (isGem(obj1)){
  		doPickupGem(obj1);
  	}
  } 
}

void updateGemCanPickup(int numBroken){
	for (auto& [id, gem] : gems){
		if (numBroken == breakables.size()){
			gem.canPickup = true;
			updateGemStyle(id);
		}
	}
}


int numBroken(){
	int count = 0;
	for (auto& [id, breakable] : breakables){
		if (breakable.isBroken){
			count++;
		}
	}
	return count;
}

void maybeBreakObject(objid id){
	if (breakables.find(id) != breakables.end()){
		auto& breakable = breakables.at(id);
		if (breakable.isBroken || !breakable.isEnabled){
			return;
		}

		breakable.isBroken = true;
		setGameObjectMeshEnabled(id, false);
		playMixedSound(getSymbol(paths::BREAKABLE), std::nullopt);

	  auto totalBroken = numBroken();
	  updateGemCanPickup(totalBroken);
	}
}
void handleBreakableCollision(objid obj1, objid obj2){
  if (isControlledPlayer(obj2) || isControlledVehicle(obj2)){
    maybeBreakObject(obj1);
  }else if (isControlledPlayer(obj1) || isControlledVehicle(obj1)){
   	maybeBreakObject(obj2);
  }
}

void disableAllBreakables(){
	for (auto& [id, breakable] : breakables){
		breakable.isEnabled = false;
		breakable.isBroken = false;
		setGameObjectTint(id, glm::vec4(1.f, 1.f, 1.f, 0.2f));
		setGameObjectMeshEnabled(id, true);
	}
	updateGemCanPickup(numBroken());
}
void enableAllBreakables(){
	for (auto& [id, breakable] : breakables){
		breakable.isEnabled = true;
		setGameObjectTint(id, glm::vec4(1.f, 1.f, 1.f, 1.f));
	}
	updateGemCanPickup(numBroken());
}