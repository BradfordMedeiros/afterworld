#include "./health.h"

extern CustomApiBindings* gameapi;
void onAiHealthChange(objid targetId, float remainingHealth);
void emitBlood(objid sceneId, objid lookAtId, glm::vec3 position);

std::unordered_map<objid, HitPoints> hitpoints = {};

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


bool doDamage(std::unordered_map<objid, HitPoints>& hitpoints, objid id, float amount, bool* _enemyDead, float* _remainingHealth){
	if (hitpoints.find(id) == hitpoints.end()){
		modlog("health", "not an enemy with tracked health: " + std::to_string(id) + ", " + gameapi -> getGameObjNameForId(id).value());
		return false;
	}
	modlog("health", "damage to: " + std::to_string(id) + ", amount = " + std::to_string(amount));

	auto activePlayerId = getActivePlayerId();

	auto enemyPos = gameapi -> getGameObjectPos(id, true);
	auto enemyRot = gameapi -> getGameObjectRotation(id, true);
	auto inFront = enemyPos + (enemyRot * glm::vec3(0.f, 0.f, -1.f));

	emitBlood(rootSceneId(), activePlayerId.value(), inFront);
	float adjustedDamageAmount = (getGlobalState().godMode && activePlayerId.has_value() && activePlayerId.value() == id) ? 0.f : amount;

	auto newHealthAmount = hitpoints.at(id).current - adjustedDamageAmount;
	hitpoints.at(id).current = newHealthAmount;
	*_enemyDead = newHealthAmount <= 0;
	*_remainingHealth = newHealthAmount;
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
  	if (allChildren.size() == 2){
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
  	gameapi -> schedule(-1, true, 1, NULL, [targetId](void*) -> void {
  		if (gameapi -> gameobjExists(targetId)){
	  		gameapi -> removeByGroupId(gameapi -> groupId(targetId));
  		}
  	});
  }
}

extern Waypoints waypoints;
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
