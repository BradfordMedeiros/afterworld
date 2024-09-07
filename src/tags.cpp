#include "./tags.h"

extern CustomApiBindings* gameapi;

struct TagUpdater {
	std::string attribute;
	std::function<void(Tags& tags, int32_t idAdded, AttributeValue value)> onAdd;
	std::function<void(Tags& tags, int32_t idAdded)> onRemove;
	std::optional<std::function<void(Tags&)>> onFrame;
	std::optional<std::function<void(Tags&, std::string& key, std::any& value)>> onMessage;
};

void handleScroll(std::set<objid>& textureScrollObjIds){
	for (auto id : textureScrollObjIds){
		modlog("tags", "scroll object: " + std::to_string(id));
		auto attrHandle = getAttrHandle(id);
		auto scrollSpeed = getVec3Attr(attrHandle, "scrollspeed").value();
		auto offset = getVec2Attr(attrHandle, "textureoffset").value();

		auto elapsedTime = gameapi -> timeElapsed();
		scrollSpeed.x *= elapsedTime;
		scrollSpeed.y *= elapsedTime;
		offset.x += scrollSpeed.x;
		offset.y += scrollSpeed.y;
		setGameObjectTextureOffset(id, offset);
	}
}

objid createPrefab(glm::vec3 position, std::string&& prefab, objid sceneId){
	std::cout << "create prefab placeholder: " << print(position) << ", " << prefab << std::endl;

  GameobjAttributes attr = {
    .attr = {
      { "scene", prefab },
			{ "position", position },
    },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes = {};
  return gameapi -> makeObjectAttr(
    sceneId, 
    std::string("[instance-") + uniqueNameSuffix(), 
    attr, 
    submodelAttributes
  ).value();
}

std::string queryInitialBackground(){
  auto query = gameapi -> compileSqlQuery("select background from session", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  return result.at(0).at(0);
}
void updateQueryBackground(std::string image){
  auto updateQuery = gameapi -> compileSqlQuery(
    std::string("update session set ") + "background = ?", { image }
  );
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(updateQuery, &validSql);
  modassert(validSql, "error executing sql query");
}
void updateBackground(objid id, std::string image){
	if (!getSingleAttr(id, "background").has_value()){
		return;
	}
  setGameObjectTexture(id, image);
}

void createExplosion(glm::vec3 position, float outerRadius, float damage){
	auto hitObjects = gameapi -> contactTestShape(position, glm::identity<glm::quat>(), glm::vec3(1.f * outerRadius, 1.f * outerRadius, 1.f * outerRadius));
	for (auto &hitobject : hitObjects){
		doDamageMessage(hitobject.id, damage);

		float force = 10.f;
		auto dirVec = glm::normalize(hitobject.point - position);
		gameapi -> applyImpulse(hitobject.id, glm::vec3(force * dirVec.x, force * dirVec.y, force * dirVec.z));
	}

	playGameplayClipById(getManagedSounds().explosionSoundObjId.value(), std::nullopt, position);

	std::cout << "hitobjects: [";
	for (auto &hitobject : hitObjects){
		std::cout << gameapi -> getGameObjNameForId(hitobject.id).value() << " ";
	}
	std::cout << "]" << std::endl;
	simpleOnFrame([position, outerRadius]() -> void {
		drawSphereVecGfx(position, outerRadius, glm::vec4(0.f, 0.f, 1.f, 1.f));
	}, 0.2f);
}

std::vector<TagUpdater> tagupdates = { 
	TagUpdater {
		.attribute = "animation",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue attrValue) -> void {
			auto value = maybeUnwrapAttrOpt<std::string>(attrValue).value();
  		auto animationController = getSingleAttr(id, "animation");
  		addEntityController(tags.animationController, id, getSymbol(animationController.value()));
  	},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
		 	removeEntityController(tags.animationController, id);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "open",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue attrValue) -> void {
 			auto value = maybeUnwrapAttrOpt<std::string>(attrValue).value();
  		tags.openable[id] = OpenableType {
  			.signal = value,
  			.closeSignal = "close-door-trigger",
  			.behavior = OPEN_BEHAVIOR_TOGGLE,
  			.stateUp = false,
  		};
  	},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
 			tags.openable.erase(id);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = [](Tags& tags, std::string& key, std::any& value) -> void {
	  	for (auto &[id, openable] : tags.openable){
	  		if (!gameapi -> gameobjExists(id)){
	  			continue;
	  		}
	  		if (key == openable.signal){
	  			if (openable.behavior == OPEN_BEHAVIOR_DELETE){
	  				auto groupId = gameapi -> groupId(id);
      			gameapi -> removeByGroupId(groupId);
	  			}else if (openable.behavior == OPEN_BEHAVIOR_UP){
	  				if (!openable.stateUp){
			  			glm::vec3 position = gameapi -> getGameObjectPos(id, true);
	  					gameapi -> setGameObjectPosition(id, position + glm::vec3(0.f, 5.f, 0.f), true);
		  				openable.stateUp = true;
	  				}
	  			}else if (openable.behavior == OPEN_BEHAVIOR_TOGGLE){
	  				if (!openable.stateUp){
			  			glm::vec3 position = gameapi -> getGameObjectPos(id, true);
	  					gameapi -> setGameObjectPosition(id, position + glm::vec3(0.f, 5.f, 0.f), true);
		  				openable.stateUp = true;
	  				}else{
			  			glm::vec3 position = gameapi -> getGameObjectPos(id, true);
	  					gameapi -> setGameObjectPosition(id, position + glm::vec3(0.f, -5.f, 0.f), true);
		  				openable.stateUp = false; 					
	  				}
	  			}else{
	  				modassert(false, "open behavior not yet implemented");
	  			}
	  		}else if (key == openable.closeSignal){
	  			if (openable.behavior == OPEN_BEHAVIOR_DELETE){
	  				// do nothing
	  			}else if (openable.behavior == OPEN_BEHAVIOR_UP){
	  				if (openable.stateUp){
			  			glm::vec3 position = gameapi -> getGameObjectPos(id, true);
	  					gameapi -> setGameObjectPosition(id, position + glm::vec3(0.f, -5.f, 0.f), true);
		  				openable.stateUp = false;
	  				}
	  			}else if (openable.behavior == OPEN_BEHAVIOR_TOGGLE){
	  				//modassert(false, "close signal for a toggle door");
	  			}else{
	  				modassert(false, "open behavior not yet implemented");
	  			}	
	  		}
	  	}
  	},
	},
	TagUpdater {
		.attribute = "scrollspeed",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue attrValue) -> void {
			auto value = maybeUnwrapAttrOpt<glm::vec3>(attrValue);
			modassert(value.has_value(), "scrollspeed not vec3");
  		//std::cout << "scroll: on object add: " << gameapi -> getGameObjNameForId(id).value() << std::endl;
  		tags.textureScrollObjIds.insert(id);
  	},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
 			tags.textureScrollObjIds.erase(id);
  	},
  	.onFrame = [](Tags& tags) -> void {
  		//std::cout << "scrollspeed: on frame: " << tags.textureScrollObjIds.size() <<  std::endl;
			handleScroll(tags.textureScrollObjIds);
  	},
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "health",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {
 			modlog("health", "entity added: " + std::to_string(id));
 			addEntityIdHitpoints(id);
  	},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
			modlog("health", "entity removed: " + std::to_string(id));
			removeEntityIdHitpoints(id);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "ambient",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {
  		//modassert(false, "on add ambient");
  		modlog("ambient", std::string("entity added") + gameapi -> getGameObjNameForId(id).value());
  		tags.audiozones.audiozoneIds.insert(id);
  		std::cout << "audio zones: " << print(tags.audiozones.audiozoneIds) << std::endl;
  	},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		modlog("ambient", std::string("entity removed") + std::to_string(id));
  		tags.audiozones.audiozoneIds.erase(id);
  		if (tags.audiozones.currentPlaying.has_value()){
  			// clip might have already been removed. 
  			gameapi -> stopClip(tags.audiozones.currentPlaying.value().clipToPlay, tags.audiozones.currentPlaying.value().sceneId);
	  		tags.audiozones.currentPlaying = std::nullopt;
  		}
  		std::cout << "audio zones: " << print(tags.audiozones.audiozoneIds) << std::endl;
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
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {
			spawnAddId(id);
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		spawnRemoveId(id);
  	},
  	.onFrame = [](Tags& tags) -> void {  
			onSpawnTick();
  	},
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "spawn-managed",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		spawnRemoveId(id);
  	},
  	.onFrame = [](Tags& tags) -> void {},
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "destroy",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		 // when this object it removed, get the position, and spawn a prefab there 
  		glm::vec3 position = gameapi -> getGameObjectPos(id, true);
  		auto sceneId = gameapi -> listSceneId(id);
  		createPrefab(position, getSingleAttr(id, "destroy").value(), sceneId);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "explode",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		 // when this object it removed, get the position, and spawn a prefab there 
  		glm::vec3 position = gameapi -> getGameObjectPos(id, true);

  		auto attrHandle = getAttrHandle(id);
			auto explodeDamage = getFloatAttr(attrHandle, "explode").value();
			auto explodeRadius = getFloatAttr(attrHandle, "explode-radius");
  		createExplosion(position, explodeRadius.has_value() ? explodeRadius.value() : 5.f, explodeDamage);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "condition",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue attrValue) -> void {
 			auto value = maybeUnwrapAttrOpt<std::string>(attrValue).value();
			onAddConditionId(id, value);
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {},
  	.onFrame = std::nullopt,
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "in-game-ui",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {
			createInGamesUiInstance(tags.inGameUi, id);
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		freeInGameUiInstance(tags.inGameUi, id);
  	},
  	.onFrame = [](Tags& tags) -> void {},
  	.onMessage = [](Tags& tags, std::string& key, std::any& value) -> void {
			if (key == "interact-ingame-ui"){
				//objid* objidPtr = std::get_if<objid>(&value);
				//modassert(objidPtr, "invalid value for interact-ingame-ui");
				if (hasTempViewpoint()){
					popTempViewpoint();
				}else{
					auto uiInstance = getAnyUiInstance(tags.inGameUi);
					if (uiInstance.has_value()){
						zoomIntoGameUi(uiInstance.value());
					}
				}
			}
  	},
	},
	TagUpdater {
		.attribute = "spin",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {
  		tags.idToRotateTimeAdded[id] = gameapi -> timeSeconds(false);
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		tags.idToRotateTimeAdded.erase(id);
  	},
  	.onFrame = [](Tags& tags) -> void {
  		for (auto &[id, time] : tags.idToRotateTimeAdded){
  			auto timeElapsed = gameapi -> timeSeconds(false) - time;
  			float degrees = (360.f * timeElapsed) * 0.2f; // 0.2f is the turns per seconds 
  			std::cout << "gun id, rotate degrees: " << degrees << std::endl;
  			float angle = glm::radians(degrees);
	  		float x = glm::cos(angle);
	  		float z = glm::sin(angle);
  			auto fromLocation = glm::vec3(0.f, 0.f, 0.f);
  			auto targetLocation = fromLocation + glm::vec3(x, 0.f, z);
  			auto newOrientation = orientationFromPos(fromLocation, targetLocation);
  			gameapi -> setGameObjectRot(id, newOrientation, true);
  		}
  	},
  	.onMessage =  std::nullopt,
	},
	TagUpdater {
		.attribute = "teleport",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {
  		tags.teleportObjs.insert(id);
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		tags.teleportObjs.erase(id);
  	},
  	.onFrame = std::nullopt,
  	.onMessage =  std::nullopt,
	},
	TagUpdater {
		.attribute = "autoplay",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {
  		playMusicClipById(id, std::nullopt, std::nullopt);
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {},
  	.onFrame = std::nullopt,
  	.onMessage =  std::nullopt,
	},
	TagUpdater {
		.attribute = "background",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {
	  	updateBackground(id, queryInitialBackground());
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {},
  	.onFrame = std::nullopt,
  	.onMessage = std::nullopt,
	},
};

void setMenuBackground(std::string background){
  auto backgrounds = gameapi -> getObjectsByAttr("background", std::nullopt, std::nullopt);
  for (auto id : backgrounds){
  	updateBackground(id, background);
  }
 	updateQueryBackground(background);
}

void onTagsMessage(Tags& tags, std::string& key, std::any& value){
  for (auto &tagUpdate : tagupdates){
  	if (tagUpdate.onMessage.has_value()){
  		tagUpdate.onMessage.value()(tags, key, value);
  	}
  }
}
void onTagsFrame(Tags& tags){
	for (auto &tagUpdate : tagupdates){
		if (tagUpdate.onFrame.has_value()){
			tagUpdate.onFrame.value()(tags);
		}
	}
}

void handleOnAddedTags(Tags& tags, int32_t idAdded){
  auto objHandle = getAttrHandle(idAdded);
	for (auto &tagUpdate : tagupdates){
    auto attrValue = getAttr(objHandle, tagUpdate.attribute.c_str());
    if (attrValue.has_value()){
	    tagUpdate.onAdd(tags, idAdded, attrValue.value());
    }
	}
}
void handleOnAddedTagsInitial(Tags& tags){
  std::set<objid> idsAlreadyExisting;
  for (auto &tagUpdate : tagupdates){
  	auto ids = gameapi -> getObjectsByAttr(tagUpdate.attribute, std::nullopt, std::nullopt);
  	for (auto idAdded : ids){
	  	idsAlreadyExisting.insert(idAdded);
  	}
  }
  for (auto idAdded : idsAlreadyExisting){
  	handleOnAddedTags(tags, idAdded);
  }
}

Tags createTags(UiContext* uiContext){
	Tags tags{};

	tags.uiContext = uiContext;

  tags.textureScrollObjIds = {};
  tags.audiozones = AudioZones {
  	.audiozoneIds = {},
  	.currentPlaying = std::nullopt,
  };
  tags.inGameUi = InGameUi {
  	.textDisplays = {},
  };
  tags.openable = {};
  tags.idToRotateTimeAdded = {};
  tags.teleportObjs = {};
  ///// animations ////
  tags.animationController = createStateController();

  addStateController(
   	tags.animationController, 
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
			}
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
  return tags;
}

void handleTagsOnObjectRemoved(Tags& tags, int32_t idRemoved){
 	for (auto &tagUpdate : tagupdates){
	  if (hasAttribute(idRemoved, tagUpdate.attribute.c_str())){
      tagUpdate.onRemove(tags, idRemoved);
    }
	}
}


std::optional<TeleportInfo> getTeleportPosition(Tags& tags){
	if (tags.teleportObjs.size() == 0){
		return std::nullopt;
	}

	auto index = randomNumber(0, tags.teleportObjs.size() - 1);
	int currIndex = 0;
	for(auto id : tags.teleportObjs){
		if (currIndex == index){
			auto position = gameapi -> getGameObjectPos(id, true);
			return TeleportInfo {
				.id = id,
				.position = position
			};
		}
		currIndex++;
	}
	return std::nullopt;
}
