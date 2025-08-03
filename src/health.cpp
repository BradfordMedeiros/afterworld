#include "./health.h"

extern CustomApiBindings* gameapi;
extern Waypoints waypoints;
extern std::unordered_map<objid, HitPoints> hitpoints ;  // static-state extern

void playGameplayClipById(objid id, std::optional<float> volume, std::optional<glm::vec3> position);
void onAiHealthChange(objid targetId, float remainingHealth);
void setIsAlive(objid id, bool alive);

void addEntityIdHitpoints(objid id){
	if (hitpoints.find(id) != hitpoints.end()){
		return;
	}
	auto totalHealth = getSingleFloatAttr(id, "health");
	if (totalHealth.has_value()){
		auto healthPoints = totalHealth.value();
		hitpoints[id] = HitPoints {
			.current = healthPoints,
			.total = healthPoints,
		};
	}
}

void removeEntityIdHitpoints(objid id){
	if (hitpoints.find(id) == hitpoints.end()){
		return;
	}
	hitpoints.erase(id);
}


void applyScreenshake(glm::vec3);

bool doDamage(std::unordered_map<objid, HitPoints>& hitpoints, objid id, float amount, bool* _enemyDead, float* _remainingHealth){
	if (hitpoints.find(id) == hitpoints.end()){
		modlog("health", "not an enemy with tracked health: " + std::to_string(id) + ", " + gameapi -> getGameObjNameForId(id).value());
		return false;
	}
	modlog("health", "damage to: " + std::to_string(id) + ", amount = " + std::to_string(amount));

	auto activePlayerId = getActivePlayerId();
	float adjustedDamageAmount = (getGlobalState().godMode && activePlayerId.has_value() && activePlayerId.value() == id) ? 0.f : amount;

	auto newHealthAmount = hitpoints.at(id).current - adjustedDamageAmount;
	hitpoints.at(id).current = newHealthAmount;
	*_enemyDead = newHealthAmount <= 0;
	*_remainingHealth = newHealthAmount;

	if (activePlayerId.has_value() && getManagedSounds().hitmarkerSoundObjId.has_value()){
		auto playerPosition = getActivePlayerPosition(); // TODO - this shoiuldn't be here, only if it is damaged by the player
		if(playerPosition.has_value()){
			//modassert(false, "hitmarker sound try play!");
			playGameplayClipById(getManagedSounds().hitmarkerSoundObjId.value(), std::nullopt, playerPosition.value()); 

			if (*_enemyDead){
				float magnitude = 10;
      	applyScreenshake(glm::vec3(magnitude * glm::cos(std::rand()), magnitude * glm::cos(std::rand()), 0.f));				
			}


		}
	}

	return true;
}

void onNoHealth(objid targetId){
  auto removeType = getSingleAttr(targetId, "health-remove");
  if (removeType.has_value() && removeType.value() == "self"){
	  modlog("health", "removing self object: " + std::to_string(targetId) + " " + gameapi -> getGameObjNameForId(targetId).value());
  	// check if the parent of the group of id has no children, if so delete it 
  	auto allChildren = gameapi -> idsInGroupById(targetId);
  	auto groupId = gameapi -> groupId(targetId);
  	gameapi -> removeObjectById(targetId);
  	if (allChildren.size() == 2){  // wtf ? 
  		modassert(false, "idk wtf this code is triggered");
  		if (allChildren.at(0) == targetId){
  			auto healthCleanup = getSingleAttr(allChildren.at(1), "health-cleanup");
  			if (healthCleanup.has_value() && healthCleanup.value() == "no-children"){
  				gameapi -> removeByGroupId(groupId);
  			}
  		}else if (allChildren.at(1) == targetId){
  			auto healthCleanup = getSingleAttr(allChildren.at(0), "health-cleanup");
  			if (healthCleanup.has_value() && healthCleanup.value() == "no-children"){
  				gameapi -> removeByGroupId(groupId);
  			}
  		}
  	}
  }else if (removeType.has_value() && removeType.value() == "kill"){
  	setIsAlive(targetId, false);
  }else{
	  modlog("health", "removing group object: " + std::to_string(targetId) + " " + gameapi -> getGameObjNameForId(targetId).value());
 		auto prefabId = gameapi -> prefabId(targetId);
 		auto idExists = gameapi -> gameobjExists(targetId);
		auto objName = gameapi -> getGameObjNameForId(targetId).value();
 		// todo why schedule?
  	gameapi -> schedule(-1, true, 1, NULL, [targetId, prefabId, idExists, objName](void*) -> void {
  		if (prefabId.has_value()){
  			//modassert(false, "should remove the whole prefab here");
  			gameapi -> removeByGroupId(prefabId.value());

  		}else{
  			modlog("health id exists", print(idExists));
  			modlog("health id name", objName);
  			//modassert(false, "object was not a prefab");
  			if (gameapi -> gameobjExists(targetId)){
	  			gameapi -> removeByGroupId(gameapi -> groupId(targetId));
  			}  			
  		}
  	});
  }
}

void doDamageMessage(objid targetId, float damageAmount){
  bool enemyDead = false;
  float remainingHealth = 0.f;
  bool valid = doDamage(hitpoints, targetId, damageAmount, &enemyDead, &remainingHealth);

  std::cout << gameapi -> getGameObjNameForId(targetId).value() << ": dead: " << enemyDead << std::endl;

  auto healthBehavior = getSingleAttr(targetId, "health-behavior");
  if (valid && enemyDead && healthBehavior.has_value() && healthBehavior.value() == "make-dynamic"){
  	setGameObjectPhysicsDynamic(targetId);
  	return;
  }
  if (valid && enemyDead){
   	onNoHealth(targetId);
  }
  onAiHealthChange(targetId, remainingHealth); // this shouldn't know about ai system

  auto hitpoints = getHealth(targetId);
  if (hitpoints.has_value()){
	  updateHealth(waypoints, targetId, hitpoints.value().current / hitpoints.value().total);
  }
}

std::optional<HitPoints> getHealth(objid id){
	if (hitpoints.find(id) == hitpoints.end()){
		return std::nullopt;
	}
	return hitpoints.at(id);
}

DebugConfig debugPrintHealth(){
  DebugConfig debugConfig { .data = {} };
	for (auto &[id, hitpoint] : hitpoints){
		debugConfig.data.push_back({ 
			gameapi -> getGameObjNameForId(id).value(), 
			std::to_string(hitpoint.current) + "    " + std::to_string(id)
		});
	}
	return debugConfig;	
}
