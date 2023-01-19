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
    addEntityIdHitpoints(tags -> hitpoints, id);
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
    	auto id = std::atoi(parts.at(1).c_str());
    	auto floatValue = std::get_if<float>(&value);
    	modassert(floatValue != NULL, "damage message but value not a float");
    	bool enemyDead = false;
    	bool valid = doDamage(*tags, id, *floatValue, &enemyDead);
    	if (valid && enemyDead){
    		gameapi -> sendNotifyMessage("onkill", std::to_string(id));
    	}
    }
  };
	return binding;
}