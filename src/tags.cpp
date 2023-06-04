#include "./tags.h"

extern CustomApiBindings* gameapi;

struct HitPoints {
	float current;
	float total;
	std::optional<std::string> eventName;
};

struct Tags {
	std::unordered_map<objid, HitPoints> hitpoints;
	std::set<objid> textureScrollObjIds;
};

struct TagUpdater {
	std::string attribute;
	attrFuncValue onAdd;
	std::function<void(void*, int32_t idAdded)> onRemove;
	std::optional<std::function<void(Tags&)>> onFrame;
	std::optional<std::function<void(Tags&, std::string& key, AttributeValue& value)>> onMessage;
};

void handleScroll(std::set<objid>& textureScrollObjIds){
	for (auto id : textureScrollObjIds){
		//modlog("tags", "scroll object: " + std::to_string(id));
		auto objAttr =  gameapi -> getGameObjectAttr(id);
		auto scrollSpeed = getVec3Attr(objAttr, "scrollspeed").value();
		auto elapsedTime = gameapi -> timeElapsed();
		scrollSpeed.x *= elapsedTime;
		scrollSpeed.y *= elapsedTime;
		auto textureOffsetStr = getStrAttr(objAttr, "textureoffset").value();
		auto offset = parseVec2(textureOffsetStr);
		offset.x += scrollSpeed.x;
		offset.y += scrollSpeed.y;
		auto textureOffset = serializeVec(offset);
		GameobjAttributes attr {
			.stringAttributes = {
				{ "textureoffset", textureOffset },
			},
			.numAttributes = {},
			.vecAttr = { .vec3 = {}, .vec4 = {} },
		};
		gameapi -> setGameObjectAttr(id, attr);
	}
}

void addEntityIdHitpoints(std::unordered_map<objid, HitPoints>& hitpoints, objid id){
	if (hitpoints.find(id) != hitpoints.end()){
		return;
	}
	auto attr = gameapi -> getGameObjectAttr(id);
	auto totalHealth = getFloatAttr(attr, "health");
	if (totalHealth.has_value()){
		auto healthPoints = totalHealth.value();
		hitpoints[id] = HitPoints {
			.current = healthPoints,
			.total = healthPoints,
			.eventName = "hud-health",
		};
	}
}

bool doDamage(Tags& tags, objid id, float amount, bool* _enemyDead, std::optional<std::string>** _eventName, float* _remainingHealth){
	if (tags.hitpoints.find(id) == tags.hitpoints.end()){
		modlog("health", "not an enemy with tracked health: " + std::to_string(id) + ", " + gameapi -> getGameObjNameForId(id).value());
		return false;
	}
	modlog("health", "damage to: " + std::to_string(id) + ", amount = " + std::to_string(amount));
	auto newHealthAmount = tags.hitpoints.at(id).current - amount;
	tags.hitpoints.at(id).current = newHealthAmount;
	*_enemyDead = newHealthAmount <= 0;
	*_eventName = &tags.hitpoints.at(id).eventName;
	*_remainingHealth = newHealthAmount;
	return true;
}

std::vector<TagUpdater> tagupdates = {
	TagUpdater {
		.attribute = "scrollspeed",
		.onAdd = [](void* data, int32_t id, glm::vec3 value) -> void {
  		Tags* tags = static_cast<Tags*>(data);
  		tags -> textureScrollObjIds.insert(id);
  	},
  	.onRemove = [](void* data, int32_t id) -> void {
 			Tags* tags = static_cast<Tags*>(data);
 			tags -> textureScrollObjIds.erase(id);
  	},
  	.onFrame = [](Tags& tags) -> void {  
			handleScroll(tags.textureScrollObjIds);
  	},
  	.onMessage = std::nullopt,
	},

	TagUpdater {
		.attribute = "health",
		.onAdd = [](void* data, int32_t id, float value) -> void {
  		Tags* tags = static_cast<Tags*>(data);
 			modlog("health", "entity added: " + std::to_string(id));
 			addEntityIdHitpoints(tags -> hitpoints, id);
  	},
  	.onRemove = [](void* data, int32_t id) -> void {
  		Tags* tags = static_cast<Tags*>(data);
			modlog("health", "entity removed: " + std::to_string(id));
			if (tags -> hitpoints.find(id) == tags -> hitpoints.end()){
				return;
			}
			tags -> hitpoints.erase(id);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = [](Tags& tags, std::string& key, AttributeValue& value) -> void {
  		auto parts = split(key, '.');
      if (parts.size() == 2 && parts.at(0) == "damage"){
      	auto targetId = std::atoi(parts.at(1).c_str());
      	modlog("health", "got damange for: " + std::to_string(targetId));
      	auto floatValue = std::get_if<float>(&value);
      	modassert(floatValue != NULL, "damage message needs to be float value");
      	bool enemyDead = false;
      	std::optional<std::string>* eventName = NULL;
      	float remainingHealth = 0.f;
      	bool valid = doDamage(tags, targetId, *floatValue, &enemyDead, &eventName, &remainingHealth);
      	if (valid && enemyDead){
      		gameapi -> sendNotifyMessage("nohealth", std::to_string(targetId));
      	}
      	if (valid && eventName -> has_value()){
      		gameapi -> sendNotifyMessage(eventName -> value(), remainingHealth);
      	}
      }else if (key == "nohealth"){
      	auto strValue = std::get_if<std::string>(&value);
      	auto targetId = std::atoi(strValue -> c_str());
      	modlog("health", "removing object: " + std::to_string(targetId));
      	gameapi -> removeObjectById(targetId);
      }
  	},
	},
};



CScriptBinding tagsBinding(CustomApiBindings& api, const char* name){
	 auto binding = createCScriptBinding(name, api);
    binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Tags* tags = new Tags;
    tags -> hitpoints = {};
    tags -> textureScrollObjIds = {};
    return tags;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Tags* tags = static_cast<Tags*>(data);
    delete tags;
  };
  binding.onMessage = attributeFn([](int32_t id, void* data, std::string& key, AttributeValue& value){
  	Tags* tags = static_cast<Tags*>(data);
  	std::cout << "tags on message: " << key << ", value = " << print(value) << std::endl;
  	for (auto &tagUpdate : tagupdates){
  		if (tagUpdate.onMessage.has_value()){
  			tagUpdate.onMessage.value()(*tags, key, value);
  		}
  	}
  });

	std::vector<AttrFunc> attrFuncs = {};
	std::vector<AttrFuncValue> attrAddFuncs = {};
	for (auto &tagUpdate : tagupdates){
		attrAddFuncs.push_back(AttrFuncValue {
			.attr = tagUpdate.attribute,
			.fn = tagUpdate.onAdd,
		});
		attrFuncs.push_back(AttrFunc {
			.attr = tagUpdate.attribute,
			.fn = tagUpdate.onRemove,
		});
	}

  binding.onObjectAdded = getOnAttrAdds(attrAddFuncs);
  binding.onObjectRemoved = getOnAttrRemoved(attrFuncs);

  binding.onFrame = [](int32_t id, void* data) -> void {
		Tags* tags = static_cast<Tags*>(data);
		for (auto &tagUpdate : tagupdates){
			if (tagUpdate.onFrame.has_value()){
				tagUpdate.onFrame.value()(*tags);
			}
		}
  };

	return binding;
}

