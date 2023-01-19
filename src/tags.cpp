#include "./tags.h"

struct HitPoints {
	float current;
	float total;
};

struct Tags {
	std::unordered_map<objid, HitPoints> hitpoints;
};

void addEntityIdHitpoints(std::unordered_map<objid, HitPoints>& hitpoints, objid id){
	hitpoints[id] = HitPoints {
		.current = 100,
		.total = 100,
	};
}
void removeEntityId(std::unordered_map<objid, HitPoints>& hitpoints, objid id){
	hitpoints.erase(id);
}
void doDamage(Tags& tags, objid id, float amount){
	if (tags.hitpoints.find(id) == tags.hitpoints.end()){
		return;
	}
	tags.hitpoints.at(id).current -= amount;
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
    	doDamage(*tags, id, *floatValue);
    }
  };
	return binding;
}