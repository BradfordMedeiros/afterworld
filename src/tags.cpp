#include "./tags.h"

extern CustomApiBindings* gameapi;

struct HitPoints {
	float current;
	float total;
	std::optional<std::string> eventName;
};

struct CurrentPlayingData {
	objid id;
	objid sceneId;
	std::string clipToPlay;
};
struct AudioZones {
	std::set<objid> audiozoneIds;
	std::optional<CurrentPlayingData> currentPlaying;
};

struct Tags {
	std::unordered_map<objid, HitPoints> hitpoints;
	std::set<objid> textureScrollObjIds;
	AudioZones audiozones;

	StateController animationController;

};

struct TagUpdater {
	std::string attribute;
	attrFuncValue onAdd;
	std::function<void(void*, int32_t idAdded)> onRemove;
	std::optional<std::function<void(Tags&)>> onFrame;
	std::optional<std::function<void(Tags&, std::string& key, std::any& value)>> onMessage;
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

	auto activePlayerId = getActivePlayerId();
	float adjustedDamageAmount = (getGlobalState().godMode && activePlayerId.has_value() && activePlayerId.value() == id) ? 0.f : amount;

	auto newHealthAmount = tags.hitpoints.at(id).current - adjustedDamageAmount;
	tags.hitpoints.at(id).current = newHealthAmount;
	*_enemyDead = newHealthAmount <= 0;
	*_eventName = &tags.hitpoints.at(id).eventName;
	*_remainingHealth = newHealthAmount;
	return true;
}


std::vector<TagUpdater> tagupdates = {
	TagUpdater {
		.attribute = "animation",
		.onAdd = [](void* data, int32_t id, std::string value) -> void {
  		Tags* tags = static_cast<Tags*>(data);
  		auto animationController = getSingleAttr(id, "animation");
  		addEntityController(tags -> animationController, id, getSymbol(animationController.value()));
  	},
  	.onRemove = [](void* data, int32_t id) -> void {
 			Tags* tags = static_cast<Tags*>(data);
		 	removeEntityController(tags -> animationController, id);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = [](Tags& tags, std::string& key, std::any& value) -> void {
  		if (key == "trigger"){
  			AnimationTrigger* animationTriggerMessage = anycast<AnimationTrigger>(value);
  			if (!hasControllerState(tags.animationController, animationTriggerMessage -> entityId)){
  				return;
  			}
        bool changedState = triggerControllerState(tags.animationController, animationTriggerMessage -> entityId, getSymbol(animationTriggerMessage -> transition));
        if (changedState){
        	auto stateAnimation = stateAnimationForController(tags.animationController, animationTriggerMessage -> entityId);
        	if (stateAnimation){
	        	modlog("animation controller", std::string("changed state to: ") + nameForSymbol(stateAnimation -> state));
        		if (stateAnimation -> animation.has_value()){
        			gameapi -> playAnimation(animationTriggerMessage -> entityId, stateAnimation -> animation.value(), stateAnimation -> animationBehavior);
        		}else{
        			gameapi -> stopAnimation(animationTriggerMessage -> entityId);
        		}
        	}
        }
  		}
  	},
	},

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
  	.onMessage = [](Tags& tags, std::string& key, std::any& value) -> void {
  		if (key == "damage"){
      	DamageMessage* dMessage = anycast<DamageMessage>(value);
	    	modassert(dMessage, "dMessage was null");

      	auto targetId = dMessage -> id;
      	modlog("health", "got damange for: " + std::to_string(targetId));
      	auto floatValue = &dMessage -> amount;
      	bool enemyDead = false;
      	std::optional<std::string>* eventName = NULL;
      	float remainingHealth = 0.f;
      	bool valid = doDamage(tags, targetId, *floatValue, &enemyDead, &eventName, &remainingHealth);
				auto objAttr =  gameapi -> getGameObjectAttr(targetId);

      	NoHealthMessage nohealth {
      		.targetId = targetId,
      		.team = getStrAttr(objAttr, "team"),
      	};


      	if (valid && enemyDead){
      		gameapi -> sendNotifyMessage("nohealth", nohealth);
      	}
      	if (valid && eventName -> has_value()){
      		gameapi -> sendNotifyMessage(eventName -> value(), remainingHealth);
      	}

				HealthChangeMessage healthMessage {
					.targetId = targetId,
					.originId = std::nullopt,
					.damageAmount = *floatValue,
					.remainingHealth = remainingHealth,
				};
     		gameapi -> sendNotifyMessage("health-change", healthMessage);

      }else if (key == "nohealth"){
      	auto nohealthMessage = anycast<NoHealthMessage>(value);
      	modassert(nohealthMessage, "nohealth target id null");
      	modlog("health", "removing object: " + std::to_string(nohealthMessage -> targetId));
      	gameapi -> removeObjectById(nohealthMessage -> targetId);
      }
  	},
	},
	TagUpdater {
		.attribute = "game-control",
		.onAdd = [](void* data, int32_t id, std::string value) -> void {},
  	.onRemove = [](void* data, int32_t id) -> void { 
  		gameapi -> sendNotifyMessage("game-over", (int)1);
  		gameapi -> sendNotifyMessage("alert", "gameover");

  	},
  	.onFrame = std::nullopt,
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "ambient",
		.onAdd = [](void* data, int32_t id, std::string value) -> void {
  		Tags* tags = static_cast<Tags*>(data);
  		//modassert(false, "on add ambient");
  		modlog("ambient", std::string("entity added") + gameapi -> getGameObjNameForId(id).value());
  		tags -> audiozones.audiozoneIds.insert(id);
  		std::cout << "audio zones: " << print(tags -> audiozones.audiozoneIds) << std::endl;
  	},
  	.onRemove = [](void* data, int32_t id) -> void {
  		Tags* tags = static_cast<Tags*>(data);
  		modlog("ambient", std::string("entity removed") + std::to_string(id));
  		tags -> audiozones.audiozoneIds.erase(id);
  		if (tags -> audiozones.currentPlaying.has_value()){
  			// clip might have already been removed. 
  			gameapi -> stopClip(tags -> audiozones.currentPlaying.value().clipToPlay, tags -> audiozones.currentPlaying.value().sceneId);
	  		tags -> audiozones.currentPlaying = std::nullopt;
  		}
  		std::cout << "audio zones: " << print(tags -> audiozones.audiozoneIds) << std::endl;
  	},
  	.onFrame = [](Tags& tags) -> void {  
  		auto transform = gameapi -> getView();
  		auto hitObjects = gameapi -> contactTestShape(transform.position, transform.rotation, glm::vec3(1.f, 1.f, 1.f));
  		std::set<objid> audioZones;
  		for (auto &hitobject : hitObjects){
  			if (tags.audiozones.audiozoneIds.count(hitobject.id) > 0){
  				audioZones.insert(hitobject.id);
  			}
  		}

  		std::optional<objid> ambientZoneDefault = std::nullopt;
  		if (audioZones.size() == 0){
  			for (auto id : tags.audiozones.audiozoneIds){
  				auto isDefault = getSingleAttr(id, "ambient_default").has_value();
  				if (isDefault){
  					ambientZoneDefault = id;
  					break;
  				}
  			}
  		}
  		if (ambientZoneDefault.has_value()){
  			audioZones.insert(ambientZoneDefault.value());
  		}

  		bool stoppedClip = false;
  		bool startedClip = false;
  		
  		if (tags.audiozones.currentPlaying.has_value()){
  			if (audioZones.count(tags.audiozones.currentPlaying.value().id) == 0){
  				// stop playing clip
  				auto currentPlaying = tags.audiozones.currentPlaying.value();
  				gameapi -> stopClip(currentPlaying.clipToPlay, currentPlaying.sceneId);
  				stoppedClip = true;
  				tags.audiozones.currentPlaying = std::nullopt;
  			}
  		}

	  	if (audioZones.size() > 0 && !tags.audiozones.currentPlaying.has_value()){
	  		auto firstId = *(audioZones.begin());
	  		auto sceneId = gameapi -> listSceneId(firstId);
	  		auto clipToPlay = getSingleAttr(firstId, "ambient").value();
	  		tags.audiozones.currentPlaying = CurrentPlayingData { .id = firstId, .sceneId = sceneId, .clipToPlay = clipToPlay };
	  		playGameplayClip(std::move(clipToPlay), sceneId, std::nullopt, std::nullopt);
	  		startedClip = true;
  		}

  		if (stoppedClip || startedClip){
  			std::cout << "ambient: started = " << (startedClip ? "true" : "false") << ", stopped = " << (stoppedClip ? "true" : "false") << ", audio zones: " << print(audioZones) << std::endl; 
  		}


  		// get the view location 
  		// contact test as ca 
  		//    --  std::vector<HitObject> (*contactTestShape)(glm::vec3 pos, glm::quat orientation, glm::vec3 scale);
  		// get the audio zone we are in, if any
  		// play that audio zone 
  	},
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "spawn",
		.onAdd = [](void* data, int32_t id, std::string value) -> void {
			spawnAddId(id);
		},
  	.onRemove = [](void* data, int32_t id) -> void {
  		spawnRemoveId(id);
  	},
  	.onFrame = [](Tags& tags) -> void {  
			onSpawnTick();
  	},
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "spawn-managed",
		.onAdd = [](void* data, int32_t id, std::string value) -> void {},
  	.onRemove = [](void* data, int32_t id) -> void {
  		spawnRemoveId(id);
  	},
  	.onFrame = [](Tags& tags) -> void {},
  	.onMessage = std::nullopt,
	},
};



CScriptBinding tagsBinding(CustomApiBindings& api, const char* name){
	  auto binding = createCScriptBinding(name, api);

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

	  auto onAddFns = getOnAttrAdds(attrAddFuncs);

    binding.create = [onAddFns](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Tags* tags = new Tags;
    tags -> hitpoints = {};
    tags -> textureScrollObjIds = {};
    tags -> audiozones = AudioZones {
    	.audiozoneIds = {},
    	.currentPlaying = std::nullopt,
    };
    
    ///// animations ////
    tags -> animationController = createStateController();

    addStateController(
    	tags -> animationController, 
    	"character",
    	{
   		  ControllerState{
					.fromState = getSymbol("idle"),
					.toState = getSymbol("walking"),
					.transition = getSymbol("walking"),
				},
   		  ControllerState{
					.fromState = getSymbol("idle"),
					.toState = getSymbol("sidestep"),
					.transition = getSymbol("sidestep"),
				},
				ControllerState{
					.fromState = getSymbol("walking"),
					.toState = getSymbol("idle"),
					.transition = getSymbol("not-walking"),
				},
				ControllerState{
					.fromState = getSymbol("walking"),
					.toState = getSymbol("jump"),
					.transition = getSymbol("jump"),
				},
				ControllerState{
					.fromState = getSymbol("sidestep"),
					.toState = getSymbol("jump"),
					.transition = getSymbol("jump"),
				},
				ControllerState{
					.fromState = getSymbol("sidestep"),
					.toState = getSymbol("idle"),
					.transition = getSymbol("not-walking"),
				},
				ControllerState{
					.fromState = getSymbol("sidestep"),
					.toState = getSymbol("walking"),
					.transition = getSymbol("walking"),
				},
				ControllerState{
					.fromState = getSymbol("walking"),
					.toState = getSymbol("sidestep"),
					.transition = getSymbol("sidestep"),
				},
   		  ControllerState{
					.fromState = getSymbol("idle"),
					.toState = getSymbol("jump"),
					.transition = getSymbol("jump"),
				},
   		  ControllerState{
					.fromState = getSymbol("jump"),
					.toState = getSymbol("idle"),
					.transition = getSymbol("land"),
				},
   		},
   		{
   			ControllerStateAnimation {
   				.state = getSymbol("idle"),
   				.animation = std::nullopt,
   				.animationBehavior = LOOP,
   			},
   			ControllerStateAnimation {
   				.state = getSymbol("walking"),
   				.animation = "walk",
   				.animationBehavior = LOOP,
   			},
   			ControllerStateAnimation {
   				.state = getSymbol("sidestep"),
   				.animation = "sidestep",
   				.animationBehavior = LOOP,
   			},
   			ControllerStateAnimation {
   				.state = getSymbol("jump"),
   				.animation = "jump",
   				.animationBehavior = FORWARDS,
   			},
   		}
   	);

    ////////////////////////////////


    std::set<objid> idsAlreadyExisting;
    for (auto &tagUpdate : tagupdates){
    	auto ids = gameapi -> getObjectsByAttr(tagUpdate.attribute, std::nullopt, std::nullopt);
    	for (auto idAdded : ids){
	    	idsAlreadyExisting.insert(idAdded);
    	}
    }
    for (auto idAdded : idsAlreadyExisting){
   		onAddFns(id, (void*)tags, idAdded);
    }

    return tags;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Tags* tags = static_cast<Tags*>(data);
    delete tags;
  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
  	Tags* tags = static_cast<Tags*>(data);
  	for (auto &tagUpdate : tagupdates){
  		if (tagUpdate.onMessage.has_value()){
  			tagUpdate.onMessage.value()(*tags, key, value);
  		}
  	}
  };

  binding.onObjectAdded = onAddFns;
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

