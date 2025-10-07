#include "./health.h"

extern CustomApiBindings* gameapi;
extern Waypoints waypoints;
extern std::unordered_map<objid, HitPoints> hitpoints ;  // static-state extern

bool enableRagdollKill = true;

void playGameplayClipById(objid id, std::optional<float> volume, std::optional<glm::vec3> position);
void onAiHealthChange(objid targetId, float remainingHealth);
void setIsAlive(objid id, bool alive);
void emitGibs(objid sceneId, objid lookAtId, glm::vec3 position);

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
	modlog("health", "damage to: " + std::to_string(id) + ", amount = " + std::to_string(amount) + " " + gameapi -> getGameObjNameForId(id).value());

	auto activePlayerId = getActivePlayerId(getDefaultPlayerIndex());
	float adjustedDamageAmount = (getGlobalState().godMode && activePlayerId.has_value() && activePlayerId.value() == id) ? 0.f : amount;

	auto newHealthAmount = hitpoints.at(id).current - adjustedDamageAmount;
	hitpoints.at(id).current = newHealthAmount;
	*_enemyDead = newHealthAmount <= 0;
	*_remainingHealth = newHealthAmount;

	if (activePlayerId.has_value() && getManagedSounds().hitmarkerSoundObjId.has_value()){
		auto playerPosition = getActivePlayerPosition(getDefaultPlayerIndex()); // TODO - this shoiuldn't be here, only if it is damaged by the player
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

void onNoHealth(objid targetId, float remainingHealth){
  auto removeType = getSingleAttr(targetId, "health-remove");

  bool shouldKillOnly = false;
  bool hasKillTag = removeType.has_value() && removeType.value() == "kill";
  if (hasKillTag){
  	modlog("health kill", std::to_string(remainingHealth));
  }
  if (hasKillTag && remainingHealth > -100.f){
  	setIsAlive(targetId, false);
  	if (enableRagdollKill){
			enterRagdoll(targetId);
  	}

  	return;
  }

  if(hasKillTag){
  	auto activePlayerId = getActivePlayerId(getDefaultPlayerIndex());
 		if (activePlayerId.has_value()){
 			auto bloodPos = gameapi -> getGameObjectPos(targetId, true, "[gamelogic] health - onNoHealth");
	  	emitGibs(rootSceneId(), activePlayerId.value(), bloodPos);
 			if (activePlayerId.value() == targetId){
 				return;
 			}
 		}
  }

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

void doDamageMessageInner(objid id, float damageAmount){
	auto targetId = id;

	auto objectName = gameapi -> getGameObjNameForId(id).value();
	auto isHead = objectName.find("Head") != std::string::npos;
	std::cout << "doDamage: is head: " << (isHead ? "true" : "false") << std::endl;
	// should check if this is part of a rig 
	// is head
	// 

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
   	onNoHealth(targetId, remainingHealth);
  }
  onAiHealthChange(targetId, remainingHealth); // this shouldn't know about ai system

  auto hitpoints = getHealth(targetId);
  if (hitpoints.has_value()){
	  updateHealth(waypoints, targetId, hitpoints.value().current / hitpoints.value().total);
  }
}

void doDamageMessage(objid id, float damageAmount){
 	auto idExists = gameapi -> gameobjExists(id);
 	if (!idExists){
 		modassertwarn(false, std::to_string(id) + " - doDamagneMessage id does not exist");
 		return;
 	}

	auto rig = handleRigHit(id);
	if (rig.has_value()){
		std::cout << "rig data: " << print(rig.value()) << std::endl;
		if (rig.value().isHeadShot){
			doDamageMessageInner(rig.value().mainId, damageAmount);
			return;
		}
	}else{
		std::cout << "rig data: not a rig" << std::endl;
	}
	doDamageMessageInner(id, damageAmount);
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
