#include "./health.h"

extern CustomApiBindings* gameapi;

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
	float adjustedDamageAmount = (getGlobalState().godMode && activePlayerId.has_value() && activePlayerId.value() == id) ? 0.f : amount;

	auto newHealthAmount = hitpoints.at(id).current - adjustedDamageAmount;
	hitpoints.at(id).current = newHealthAmount;
	*_enemyDead = newHealthAmount <= 0;
	*_remainingHealth = newHealthAmount;
	return true;
}

void onNoHealth(objid targetId){
  modlog("health", "removing object: " + std::to_string(targetId));
  auto removeType = getSingleAttr(targetId, "health-remove");
  if (removeType.has_value() && removeType.value() == "self"){
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
	 	gameapi -> removeByGroupId(targetId);
  }
}

void doDamageMessage(objid targetId, float damageAmount){
  bool enemyDead = false;
  float remainingHealth = 0.f;
  bool valid = doDamage(hitpoints, targetId, damageAmount, &enemyDead, &remainingHealth);

  auto healthBehavior = getSingleAttr(targetId, "health-behavior");
  if (valid && enemyDead && healthBehavior.has_value() && healthBehavior.value() == "make-dynamic"){
  	setGameObjectPhysicsDynamic(targetId);
  	return;
  }
  if (valid && enemyDead){
   	onNoHealth(targetId);
  }

	HealthChangeMessage healthMessage {
		.targetId = targetId,
		.originId = std::nullopt,
		.damageAmount = damageAmount,
		.remainingHealth = remainingHealth,
	};
  gameapi -> sendNotifyMessage("health-change", healthMessage);
}

std::optional<HitPoints> getHealth(objid id){
	if (hitpoints.find(id) == hitpoints.end()){
		return std::nullopt;
	}
	return hitpoints.at(id);
}