#include "./tags.h"

extern CustomApiBindings* gameapi;

struct HitPoints {
	float current;
	float total;
};

struct Tags {
	std::unordered_map<objid, HitPoints> hitpoints;
};

void addEntityIdHitpoints(std::unordered_map<objid, HitPoints>& hitpoints, objid id){
	auto attr = gameapi -> getGameObjectAttr(id);
	auto totalHealth = getFloatAttr(attr, "health");
	if (totalHealth.has_value()){
		auto healthPoints = totalHealth.value();
		hitpoints[id] = HitPoints {
			.current = healthPoints,
			.total = healthPoints,
		};
	}
}
void removeEntityId(std::unordered_map<objid, HitPoints>& hitpoints, objid id){
	if (hitpoints.find(id) == hitpoints.end()){
		return;
	}
	hitpoints.erase(id);
}
bool doDamage(Tags& tags, objid id, float amount, bool* _enemyDead){
	if (tags.hitpoints.find(id) == tags.hitpoints.end()){
		return false;
	}
	modlog("health", "damage to: " + std::to_string(id) + ", amount = " + std::to_string(amount));
	auto newHealthAmount = tags.hitpoints.at(id).current - amount;
	tags.hitpoints.at(id).current = newHealthAmount;
	*_enemyDead = newHealthAmount <= 0;
	return true;
}

CScriptBinding tagsBinding(CustomApiBindings& api, const char* name){
	 auto binding = createCScriptBinding(name, api);
    binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Tags* tags = new Tags;
    tags -> hitpoints = {};

    auto managedEnemies = gameapi -> getObjectsByAttr("health", std::nullopt, std::nullopt);
    for (auto id : managedEnemies){
    	modlog("health", "adding id: " + std::to_string(id));
    	addEntityIdHitpoints(tags -> hitpoints, id);
    }
    return tags;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Tags* value = (Tags*)data;
    delete value;
  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, AttributeValue& value){
    auto parts = split(key, '.');
    if (parts.size() == 2 && parts.at(0) == "damage"){
    	Tags* tags = static_cast<Tags*>(data);
    	auto targetId = std::atoi(parts.at(1).c_str());
    	modlog("health", "got damange for: " + std::to_string(targetId));
    	auto floatValue = std::get_if<float>(&value);
    	modassert(floatValue != NULL, "damage message needs to be float value");
    	bool enemyDead = false;
    	bool valid = doDamage(*tags, targetId, *floatValue, &enemyDead);
    	if (valid && enemyDead){
    		gameapi -> sendNotifyMessage("onkill", std::to_string(targetId));
    	}
    }else if (key == "onkill"){
    	auto strValue = std::get_if<std::string>(&value);
    	auto targetId = std::atoi(strValue -> c_str());
    	modlog("health", "removing object: " + targetId);
    	gameapi -> removeObjectById(targetId);
    }
  };
	return binding;
}