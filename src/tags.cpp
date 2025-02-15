#include "./tags.h"

extern CustomApiBindings* gameapi;

extern Tags tags;
extern Waypoints waypoints;
extern ArcadeApi arcadeApi;
extern Director director;

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
  setGameObjectTexture(id, image);
}

struct Signal {
	bool locked;   // should remove on unloading 
};
std::map<std::string, Signal> lockedSignals {};

bool isSignalLocked(std::string signal){
	if(lockedSignals.find(signal) == lockedSignals.end()){
		return false;
	}
	return lockedSignals.at(signal).locked;
};
// RECORDING_PLAY_ONCE, RECORDING_PLAY_LOOP 
// RECORDING_PLAY_ONCE_REVERSE
void playRecordingBySignal(std::string signal, std::string rec, bool reverse){
	for (auto &[id, recording] : tags.recordings){
		if (recording.signal == signal){
			auto length = gameapi -> recordingLength(rec);
			gameapi -> playRecording(id, rec, reverse ? RECORDING_PLAY_ONCE_REVERSE : RECORDING_PLAY_ONCE, std::nullopt);
			modlog("play recording playback length", std::to_string(length));
			lockedSignals[signal] = Signal {
				.locked = true,
			};
  		gameapi -> schedule(-1, true, length * 1000, NULL, [id, signal](void*) -> void {
				modlog("play recording playback stopped", "done");
				lockedSignals.erase(signal);
  		});
		}
	}
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
	emitExplosion(rootSceneId(), getActivePlayerId().value(), position, glm::vec3(1.f, 1.f, 1.f));

	std::cout << "hitobjects: [";
	for (auto &hitobject : hitObjects){
		std::cout << gameapi -> getGameObjNameForId(hitobject.id).value() << " ";
	}
	std::cout << "]" << std::endl;
	simpleOnFrame([position, outerRadius]() -> void {
		drawSphereVecGfx(position, outerRadius, glm::vec4(0.f, 0.f, 1.f, 1.f));
	}, 0.2f);
}


struct GlassTexture {
	objid id;
	std::string name;
};
std::unordered_map<objid, GlassTexture> objIdToGlassTexture;

void createGlassTexture(objid id){
	std::string textureName = std::string("glass-texture") + uniqueNameSuffix();
	auto glassTextureId = gameapi -> createTexture(textureName, 1000, 10000, id);
	objIdToGlassTexture[id] = GlassTexture {
		.id = glassTextureId,
		.name = textureName,
	};
  gameapi -> drawRect(0.f /*centerX*/, 0.f /*centerY*/, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 0.75f), glassTextureId, true, std::nullopt, "./res/textures/water.jpg", std::nullopt);
 	updateBackground(id, objIdToGlassTexture.at(id).name);
}
void removeGlassTexture(objid id){
	auto textureName = objIdToGlassTexture.at(id).name;
	gameapi -> freeTexture(textureName, id);
	objIdToGlassTexture.erase(id);
}

// This should add normals, and drawRect should be able to subtractively add or something like that, so that
// can make the drawRect call subtract alpha
//
// This also only worked for glass the player is looking at...which is okay enough, but lame
// probably could do a quick render on  the object or something to map pos/normal of hit to uv space of object
bool maybeAddGlassBulletHole(objid id, objid playerId){
	if (objIdToGlassTexture.find(id) == objIdToGlassTexture.end()){
		return false;
	}
	GlassTexture& glassTexture = objIdToGlassTexture.at(id);
  auto ndiCoord = uvToNdi(getGlobalState().texCoordUvView);
	float bulletHoleSize = randomNumber(0.f, 0.1f) + 0.1;

	 gameapi -> drawRect(ndiCoord.x /*centerX*/, ndiCoord.y /*centerY*/, bulletHoleSize, bulletHoleSize, false, glm::vec4(1.f, 1.f, 1.f, 0.8f), glassTexture.id, true, std::nullopt, "./res/textures/glassbroken.png", std::nullopt);
	return true;
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
		.attribute = "recording",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue attrValue) -> void {
			auto signal = maybeUnwrapAttrOpt<std::string>(attrValue).value();
 			tags.recordings[id] = ManagedRecording{
 				.signal = signal,
 			};
  	},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		tags.recordings.erase(id);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "switch",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue attrValue) -> void {
			auto onSignal = getSingleAttr(id, "switch");
			auto offSignal = getSingleAttr(id, "switch-reverse");
			addSwitch(tags.switches, id, onSignal, offSignal);
  	},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		removeSwitch(tags.switches, id);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "switch-reverse",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue attrValue) -> void {
			auto onSignal = getSingleAttr(id, "switch");
			auto offSignal = getSingleAttr(id, "switch-reverse");
			addSwitch(tags.switches, id, onSignal, offSignal);
  	},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		removeSwitch(tags.switches, id);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = std::nullopt,
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
			spawnAddId(director.managedSpawnpoints, id);
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		spawnRemoveId(director.managedSpawnpoints, id);
  	},
  	.onFrame = [](Tags& tags) -> void {  
			onSpawnTick(director.managedSpawnpoints);
  	},
  	.onMessage = [](Tags& tags, std::string& key, std::any& value) -> void {},
	},
	TagUpdater {
		.attribute = "spawn-managed",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {
			modlog("spawn-manage added: ", gameapi -> getGameObjNameForId(id).value());
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		spawnRemoveId(director.managedSpawnpoints, id);
  		modassert(false, "spawn-manage removed");
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
	TagUpdater {
		.attribute = "glasstexture",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {
	  	createGlassTexture(id);
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		removeGlassTexture(id);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "waypoint",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {
	  	addWaypoint(waypoints, id, id);
	  	auto hitpoints = getHealth(id);
	  	if (hitpoints.has_value()){
	  		updateHealth(waypoints, id, hitpoints.value().current / hitpoints.value().total);
	  	}
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		removeWaypoint(waypoints, id);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "autoemission",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue value) -> void {
			auto period = maybeUnwrapAttrOpt<float>(value).value();
	  	auto attrHandle = getAttrHandle(id);
			auto emissionLow = getVec3Attr(attrHandle, "autoemission-low");
			auto emissionHigh = getVec3Attr(attrHandle, "autoemission-high");

	  	tags.emissionObjects[id] = EmissionObject {
	  		.lowColor = emissionLow.has_value() ? emissionLow.value(): glm::vec3(0.f, 0.f, 0.f),
	  		.highColor = emissionHigh.has_value() ? emissionHigh.value() : glm::vec3(1.f, 1.f, 1.f),
	  		.period = period,
	  	};
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		tags.emissionObjects.erase(id);
  	},
  	.onFrame = [](Tags& tags) -> void {
  		for (auto &[id, emissionObject] : tags.emissionObjects){
				float integer = 0.f;
				float remaining = std::modf(gameapi -> timeSeconds(false) / emissionObject.period, &integer);
				float interp = remaining < 0.5f ? (remaining * 2.f) : (1.f - ((remaining - 0.5f) * 2.f));
 				modlog("emission object interp", std::to_string(interp));

	  		glm::vec3 newColor(
	  			emissionObject.lowColor.r + ((emissionObject.highColor.r - emissionObject.lowColor.r) * interp), 
	  			emissionObject.lowColor.g + ((emissionObject.highColor.g - emissionObject.lowColor.g) * interp), 
	  			emissionObject.lowColor.b + ((emissionObject.highColor.b - emissionObject.lowColor.b) * interp)
	  		);
	  		setGameObjectEmission(id, newColor);
  		}
  	},
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "cutscene",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue value) -> void {
			auto cutscene = getSingleAttr(id, "cutscene");
			playCutscene(id, cutscene.value(), gameapi -> timeSeconds(false));
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
	    onCutsceneObjRemoved(id);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "arcade",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue value) -> void {
			modlog("arcade", "on add");
			std::string textureName = std::string("arcade-texture") + std::to_string(id);
			auto arcadeTextureId = gameapi -> createTexture(textureName, 1000, 1000, id);
	 	  gameapi -> drawRect(0.f /*centerX*/, 0.f /*centerY*/, 2.f, 2.f, false, glm::vec4(1.f, 0.f, 1.f, 0.75f), arcadeTextureId, true, std::nullopt, "./res/textures/water.jpg", std::nullopt);
		 	updateBackground(id, textureName);

  		auto arcadeType = getSingleAttr(id, "arcade");
			addArcadeType(id, arcadeType.value(), arcadeTextureId);
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
			std::string textureName = std::string("arcade-texture") + std::to_string(id);
 			gameapi -> freeTexture(textureName, id);
			maybeRemoveArcadeType(id);
			unloadManagedSounds(id);
			unloadManagedTexturesLoaded(id);
  	},
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

Tags createTags(UiData* uiData){
	Tags tags{};
	tags.uiData = uiData;
  tags.textureScrollObjIds = {};
  tags.audiozones = AudioZones {
  	.audiozoneIds = {},
  	.currentPlaying = std::nullopt,
  };
  tags.inGameUi = InGameUi {
  	.textDisplays = {},
  };
  tags.idToRotateTimeAdded = {};
  tags.emissionObjects = {};
  tags.teleportObjs = {};
  tags.recordings = {};
  tags.switches = Switches{
  	.switches = {},
  };
  ///// animations ////
  tags.animationController = createStateController();
  addAnimationController(tags.animationController);
  
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
