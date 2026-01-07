#include "./tags.h"

extern CustomApiBindings* gameapi;

extern Tags tags;
extern Waypoints waypoints;
extern ArcadeApi arcadeApi;
extern Director director;
extern Vehicles vehicles;
extern GameTypes gametypeSystem;
extern OrbData orbData;
extern std::optional<std::string> activeLevel;
extern std::unordered_map<objid, LinePoints> rails;
extern std::unordered_map<objid, ManagedRailMovement> managedRailMovements;

void applyImpulseAffectMovement(objid id, glm::vec3 force);
std::optional<objid> findChildObjBySuffix(objid id, const char* objName);
void enterVehicleRaw(int playerIndex, objid vehicleId, objid id);
void setCanExitVehicle(bool canExit);
void goToLevel(std::string levelShortName);
void goBackMainMenu();

struct TagUpdater {
	std::string attribute;
	std::function<void(Tags& tags, int32_t idAdded, AttributeValue value)> onAdd;
	std::function<void(Tags& tags, int32_t idAdded)> onRemove;
	std::optional<std::function<void(Tags&)>> onFrame;
	std::optional<std::function<void(Tags&, std::string& key, std::any& value)>> onMessage;
};

bool isInGameMode2(){
	return getGlobalState().routeState.inGameMode;	
}

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
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes = {};
  return gameapi -> makeObjectAttr(
    sceneId, 
    std::string("[instance-") + uniqueNameSuffix(), 
    attr, 
    submodelAttributes
  ).value();
}


std::string queryInitialBackground(){
	if (hasOption("background")){
		return getArgOption("background");
	}
	return getSaveStringValue("settings", "background", "../gameresources/textures/backgrounds/test3.png");
}
void updateQueryBackground(std::string image){
	persistSave("settings", "background", image);
}
void updateBackground(objid id, std::string image){
  setGameObjectTexture(id, image);
}

struct Signal {
	bool locked;   // should remove on unloading 
};
std::unordered_map<std::string, Signal> lockedSignals {};

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

void toggleAutodoor(objid id, Autodoor& autodoor){
  auto targetId = findChildObjBySuffix(id, "gate");
  modassert(targetId.has_value(), "target not found for autodoor");
  auto reverse = autodoor.open ?  RECORDING_PLAY_ONCE_REVERSE : RECORDING_PLAY_ONCE;
  autodoor.open = !autodoor.open;
  gameapi -> playRecording(targetId.value(), "../afterworld/data/recordings/move-gate.rec", reverse, RecordingOptionResume{});
}



void createExplosion(glm::vec3 position, float outerRadius, float damage){
	auto hitObjects = gameapi -> contactTestShape(position, glm::identity<glm::quat>(), glm::vec3(1.f * outerRadius, 1.f * outerRadius, 1.f * outerRadius));
	for (auto &hitobject : hitObjects){
		doDamageMessage(hitobject.id, damage);

		float force = 30.f;
		auto dirVec = glm::normalize(hitobject.point - position);
		applyImpulseAffectMovement(hitobject.id, glm::vec3(force * dirVec.x, force * dirVec.y, force * dirVec.z));
	}

	playGameplayClipById(getManagedSounds().explosionSoundObjId.value(), std::nullopt, position);
	auto activePlayer = getActivePlayerId(getDefaultPlayerIndex());
	if (activePlayer.has_value()){
		emitExplosion(rootSceneId(), activePlayer.value(), position, glm::vec3(1.f, 1.f, 1.f));
	}

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

void setBallLevelComplete(){
	changeGameType(gametypeSystem, NULL, NULL);
	markLevelComplete(activeLevel.value(), true);
	goToLevel("ballselect");
}

struct Skippable {
	std::optional<std::string> advanceToLevel;
};
std::unordered_map<objid, Skippable> skippable;

std::vector<TagUpdater> tagupdates = { 
	TagUpdater {
		.attribute = "animation",  // TODO this should probably move to entity
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
  		if (!isInGameMode2()){
  			return;
  		}
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
  		// TODO perframe
  		// This doesn't need to be per frame, can easily make this on the collision callbacks
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
  		if (!isInGameMode2()){
  			return;
  		}
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
  	},
  	.onFrame = [](Tags& tags) -> void {},
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "destroy",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		 // when this object it removed, get the position, and spawn a prefab there 
  		if (!isInGameMode2()){
				return;
  		}
  		glm::vec3 position = gameapi -> getGameObjectPos(id, true, "[gamelogic] tags - destroy");
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
  		if (!isInGameMode2()){
  			return;
  		}
 		  // when this object it removed, get the position, and spawn a prefab there 
  		glm::vec3 position = gameapi -> getGameObjectPos(id, true, "[gamelogic] tags - explode");

  		auto attrHandle = getAttrHandle(id);
			auto explodeDamage = getFloatAttr(attrHandle, "explode").value();
			auto explodeRadius = getFloatAttr(attrHandle, "explode-radius");
  		createExplosion(position, explodeRadius.has_value() ? explodeRadius.value() : 5.f, explodeDamage);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "damageafter",  // make this remove-after and then can just use the explode
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {
			// do not do it this way...
  		auto attrHandle = getAttrHandle(id);
			auto explodeAfterSeconds = getFloatAttr(attrHandle, "damageafter").value();
			tags.explosionObjects[id] = ExplosionObj {
				.time = gameapi -> timeSeconds(false) + explodeAfterSeconds,
			};
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		tags.explosionObjects.erase(id);
  	},
  	.onFrame = [](Tags& tags) -> void {
   		auto currTime = gameapi -> timeSeconds(false);

   		std::vector<objid> explodedObjs;
   		for (auto &[id, explosionObj] : tags.explosionObjects){
  			if (currTime >= explosionObj.time){
  				explodedObjs.push_back(id);
					doDamageMessage(id, 100.f);
  			}
  		}
  		for (auto id : explodedObjs){
  			tags.explosionObjects.erase(id);
  		}
  	},
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "linkorb", 
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {
			tags.linkGunObj[id] = LinkGunObj {
			};
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		tags.linkGunObj.erase(id);
  		for (auto &[id, linkObj] : tags.linkGunObj){
  			doDamageMessage(id, 1000.f);
  		}
  	},
  	.onFrame = [](Tags& tags) -> void {
  		// check the nodes, if 
  		for (auto &[id1, linkObj1] : tags.linkGunObj){
	  		for (auto &[id2, linkObj2] : tags.linkGunObj){
  				if (id1 == id2){
  					continue;
  				}
					auto pos1 = gameapi -> getGameObjectPos(id1, true, "[gamelogic] tags - linkorb positions");
 					auto pos2 = gameapi -> getGameObjectPos(id2, true, "[gamelogic] tags - linkorb positions");
 					if (pos1.x < pos2.x){  // just so we only draw one connection between each, arbitrary function
  		  		gameapi -> drawLine(pos1, pos2, false, id1, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt, std::nullopt);
  		  	}
	  		}
	  	}

	  	for (auto &[id, _] : tags.linkGunObj){
				auto pos1 = gameapi -> getGameObjectPos(id, true, "[gamelogic] tags - link orb - vert lines");
	  		gameapi -> drawLine(pos1, pos1 + glm::vec3(0.f, 0.4f, 0.f), false, id, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);

	  	}
  	},
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
  		if (!isInGameMode2()){
  			return;
  		}
  		for (auto &[id, time] : tags.idToRotateTimeAdded){
  			auto timeElapsed = gameapi -> timeSeconds(false) - time;
  			float degrees = (360.f * timeElapsed) * 0.02f; // 0.2f is the turns per seconds 
  			std::cout << "gun id, rotate degrees: " << degrees << std::endl;
  			float angle = glm::radians(degrees);
	  		float x = glm::cos(angle);
	  		float z = glm::sin(angle);
  			auto fromLocation = glm::vec3(0.f, 0.f, 0.f);
  			auto targetLocation = fromLocation + glm::vec3(x, 0.f, z);
  			auto newOrientation = orientationFromPos(fromLocation, targetLocation);
  			gameapi -> setGameObjectRot(id, newOrientation, true, Hint { .hint = "tags - spin" }); // tempchecked
  		}
  	},
  	.onMessage =  std::nullopt,
	},
	TagUpdater {
		.attribute = "teleport",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue) -> void {
	  	auto attrHandle = getAttrHandle(id);
			auto teleportExit = getStrAttr(attrHandle, "teleport_exit");
  		tags.teleportObjs[id] = TeleportExit{
  			.exit = teleportExit,
  		};
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
		.attribute = "healthcolor",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue value) -> void {
	  	auto attrHandle = getAttrHandle(id);
			auto colorLow = getVec3Attr(attrHandle, "healthcolor-low");
			auto colorHigh = getVec3Attr(attrHandle, "healthcolor");
			auto colorTarget = getStrAttr(attrHandle, "healthcolor-target");
			auto target = colorTarget.has_value() ? findBodyPart(id, colorTarget.value().c_str()) : std::nullopt;

	  	tags.healthColorObjects[id] = HealthColorObject {
	  		.lowColor = colorLow.has_value() ? colorLow.value(): glm::vec3(0.f, 0.f, 0.f),
	  		.highColor = colorHigh.has_value() ? colorHigh.value() : glm::vec3(1.f, 1.f, 1.f),
	  		.target = target,
	  	};
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		tags.healthColorObjects.erase(id);
  	},
  	.onFrame = [](Tags& tags) -> void {
  		if (!isInGameMode2()){
  			return;
  		}
  		for (auto &[id, healthColorObject] : tags.healthColorObjects){
				auto health = getHealth(id);
				float percentageHealth = health.value().current / health.value().total;
	  		glm::vec4 newColor(
	  			healthColorObject.lowColor.r + ((healthColorObject.highColor.r - healthColorObject.lowColor.r) * percentageHealth), 
	  			healthColorObject.lowColor.g + ((healthColorObject.highColor.g - healthColorObject.lowColor.g) * percentageHealth), 
	  			healthColorObject.lowColor.b + ((healthColorObject.highColor.b - healthColorObject.lowColor.b) * percentageHealth),
	  			1.f
	  		);
	  		if (healthColorObject.target.has_value()){
		  		setGameObjectTint(healthColorObject.target.value(), newColor);
	  		}else{
		  		setGameObjectTint(id, newColor);
	  		}
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
	TagUpdater {
		.attribute = "autodoor",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue value) -> void {
			auto toggleSignal = std::get_if<std::string>(&value);
			tags.autodoors[id] = Autodoor{
				.open = false,
				.toggleSignal = toggleSignal ? *toggleSignal : std::string(""),
			};			
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		tags.autodoors.erase(id);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = [](Tags& tags, std::string& key, std::any& value) -> void {
  		if (key == "open-autodoor"){
	  		auto valueStr = anycast<MessageWithId>(value);
	  		modassert(valueStr, "open-autodoor should be MessageWithId type");
  			for (auto &[id, autodoor] : tags.autodoors){
  				if (valueStr -> value.has_value() && autodoor.toggleSignal == valueStr -> value.value()){
	  				toggleAutodoor(id, autodoor);
  				}
  			}
  		}
  	},
	},

	TagUpdater {
		.attribute = "globallight",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue value) -> void {
			gameapi -> setGlobalLight(id);
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  	},
  	.onFrame = std::nullopt,
  	.onMessage = [](Tags& tags, std::string& key, std::any& value) -> void {
  	},
	},

	TagUpdater {
		.attribute = "orbui",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue value) -> void {
			orbData.orbUis[id] = createOrbUi(id);			
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		orbData.orbUis.erase(id);
  	},
  	.onFrame = [](Tags& tags) -> void {
 			std::set<objid> cachesToRemove;
			for (auto& [orbUiId, indexToMesh] : orbData.orbIdToIndexToMeshId){
				if (indexToMesh.size() == 0 || orbData.orbUis.find(orbUiId) == orbData.orbUis.end()){
					cachesToRemove.insert(orbUiId);
				}
			}
			for (auto cacheToRemove : cachesToRemove){
				for (auto&[index, meshObjId] :  orbData.orbIdToIndexToMeshId.at(cacheToRemove)){
					gameapi -> removeObjectById(meshObjId);
				}
				orbData.orbIdToIndexToMeshId.erase(cacheToRemove);
			}

			for (auto& [id, orbUi] : orbData.orbUis){
				auto objExists = gameapi -> gameobjExists(id);
				if (!objExists){
					continue;
				}
	  		drawOrbs(orbData, orbUi, id);  			
			}
  	},
  	.onMessage = [](Tags& tags, std::string& key, std::any& value) -> void {
  	},
	},
	TagUpdater {
		.attribute = "orbview",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue value) -> void {
			std::string* strValue = std::get_if<std::string>(&value);
			modassert(strValue, "orbview target not a thing");
			auto orbId = findObjByShortName(*strValue, std::nullopt);
			modassert(orbId.has_value(), "orbview target does not exist");
			orbData.orbViewsCameraToOrb[id] = OrbView {
				.orbId = orbId.value(),
				.actualIndex = 0,
				.targetIndex = 0,
				.startTime = std::nullopt,
			};
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		orbData.orbViewsCameraToOrb.erase(id);
  	},
  	.onFrame = [](Tags& tags) -> void {
  		handleOrbViews(orbData);
  		std::cout << "active level: " << print(activeLevel) << std::endl;
  	},
  	.onMessage = [](Tags& tags, std::string& key, std::any& value) -> void {
  	},
	},
	

	TagUpdater {
		.attribute = "vehicle",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue value) -> void {
			auto vehicleType = std::get_if<std::string>(&value);
			modassert(vehicleType, "vehicle type not defined as str");
			if (*vehicleType == "" || *vehicleType == "ship"){
				addVehicle(vehicles, id, true);
			}else if (*vehicleType == "ball"){
				addVehicle(vehicles, id, false);
			}else{
				modassert(false, std::string("vehicle type not supported: ") + *vehicleType);
			}
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		removeVehicle(vehicles, id);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = [](Tags& tags, std::string& key, std::any& value) -> void {
  	},
	},
	// Is this duplicate with the end level thing?
	TagUpdater {
		.attribute = "advance",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue value) -> void {
 			auto advanceToLevel = getSingleAttr(id, "advance");
			skippable[id] = Skippable{
				.advanceToLevel = advanceToLevel,
			};
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
 			auto levelComplete = getSingleAttr(id, "markcomplete");
 			if (levelComplete.has_value()){
 				markLevelComplete(levelComplete.value(), true);
 			}
 			skippable.erase(id);
  	},
  	.onFrame = [](Tags& tags) -> void {

  	},
  	.onMessage = [](Tags& tags, std::string& key, std::any& value) -> void {
  		if (key == "advance"){
  			for (auto& [id, skip] : skippable){
  				if (skip.advanceToLevel.has_value()){
	  				goToLevel(skip.advanceToLevel.value());
  				}else{
	  				goBackMainMenu();
  				}
  				return;
  			}
  		}
  	},
	},

	TagUpdater {
		.attribute = "mode",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue value) -> void {
			auto modeStr = std::get_if<std::string>(&value);
			modassert(modeStr, "modeStr not a string");
			if (*modeStr == "ball"){
				auto level = getSingleAttr(id, "ball-level");
				auto playerSpawnId = findObjByShortName("playerspawn", std::nullopt);
				auto position = gameapi -> getGameObjectPos(playerSpawnId.value(), true, "[gamelogic] ball - get playerspawn position");

				createBallObj(gameapi -> listSceneId(id), position);

				if (level.has_value()){
					createLevelObj(gameapi -> listSceneId(id), level.value());
				}

				BallModeOptions modeOptions {};

				modeOptions.getBallId = []() -> std::optional<objid> {
				  auto vehicles = getVehicleIds();
 				  modassert(vehicles.size() == 1, "invalid expected vehicle size");
					return vehicles.at(0);
				};
				modeOptions.setPlayerControl = [](std::function<void()> fn) -> void {
  				gameapi -> schedule(0, true, 1, NULL, [fn](void*) -> void {
					  auto activePlayer = getActivePlayerId(0);
					  auto vehicles = getVehicleIds();
					  modassert(vehicles.size() == 1, "invalid expected vehicle size");
					  std::cout << "vehicles: " << print(vehicles) << "[" << gameapi -> getGameObjNameForId(vehicles.at(0)).value()  << "]" << ", playerid: " << print(activePlayer) << ", [" << gameapi -> getGameObjNameForId(activePlayer.value()).value()  <<  "]" << std::endl;
					  std::cout << "vehicles: " << gameapi -> getGameObjNameForId(gameapi -> getActiveCamera(std::nullopt).value()).value()  << std::endl;
					  enterVehicleRaw(0, vehicles.at(0), activePlayer.value());
					  setCanExitVehicle(false);
					  fn();
  				});
				};
				modeOptions.changeUi = [](bool showBallUi) -> void {
					if (showBallUi){
						setShowBallOptions(
							BallComponentOptions {
								//.winMessage = "you win congrats",
								.powerupTexture = "../gameresources/build/textures/ballgame/jump.png",

							}
						);
					}else{
						setShowBallOptions(std::nullopt);
					}
				};
				modeOptions.showTimeElapsed = [](std::optional<float> startTime) -> void {
					if (!showBallOptions().has_value()){
						return;
					}
					auto ballOptions = showBallOptions().value();
					ballOptions.startTime = startTime;
					setShowBallOptions(ballOptions);
				};
				modeOptions.setLevelFinished = []() -> void {
					setBallLevelComplete();
				};
				modeOptions.getPowerup = []() -> std::optional<BallPowerup> {
					auto activePlayer = getActivePlayerId(0);
					auto vehicleIds = getVehicleIds();
					modassert(vehicleIds.size() == 1, "invalid expected vehicle size");
					return getBallPowerup(vehicles, vehicleIds.at(0));
				};
				modeOptions.setPowerupTexture = [](std::string texture) -> void {
					auto ballOptions = showBallOptions();
					if (!ballOptions.has_value()){
						modassert(false, "no ballOptions");
						return;
					}
					if (ballOptions.has_value()){
						auto newBallOptions = ballOptions.value();
						newBallOptions.powerupTexture = texture;;
						setShowBallOptions(newBallOptions);
					}
				};

				changeGameType(gametypeSystem, "ball", &modeOptions);
			}else if (*modeStr == "orb-select"){
				auto cameraId = findObjByShortName(">camera-view", gameapi -> listSceneId(id));
				setTempCamera(cameraId.value(), 0);
			}else{
				modassert(false, "invalid mode type");
			}
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		setCanExitVehicle(true);
  		setShowBallOptions(std::nullopt);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = [](Tags& tags, std::string& key, std::any& value) -> void {
  	},
	},

	TagUpdater {
		.attribute = "powerup",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue value) -> void {
	    auto powerup = getSingleAttr(id, "powerup");
	    auto respawnRate = getSingleFloatAttr(id, "powerup-rate");

	    std::optional<int> respawnRateMs;
	    if (respawnRate.has_value()){
	    	respawnRateMs = static_cast<int>(respawnRate.value());
	    }

			tags.powerups[id] = Powerup {
				.type = powerup.value(),
				.respawnRateMs = respawnRateMs,
				.disabledVisually = false,
			};
		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		tags.powerups.erase(id);
  	},
  	.onFrame = [](Tags& tags) -> void {
  		float currTime = gameapi -> timeSeconds(false);
  		for (auto& [id, powerup] : tags.powerups){
  			if (powerup.lastRemoveTime.has_value() && !powerup.disabledVisually){
    			setGameObjectTint(id, glm::vec4(1.f, 1.f, 1.f, 0.2f));
    			powerup.disabledVisually = true;
  			}
  			if (!powerup.lastRemoveTime.has_value() && powerup.disabledVisually){
    			setGameObjectTint(id, glm::vec4(1.f, 1.f, 1.f, 1.f));
    			powerup.disabledVisually = false;
  			}

  			if (!powerup.lastRemoveTime.has_value()){
  				continue;
  			}
  			if (!powerup.respawnRateMs.has_value()){
  				continue;
  			}

  			float respawnTime = powerup.respawnRateMs.value() / 1000.f;
  			auto elapsedTime = currTime - powerup.lastRemoveTime.value();

  			std::cout << "powerup: currtime = " << currTime << ", elapsedTime = " << elapsedTime << ", lastremove = " << powerup.lastRemoveTime.value() << std::endl;

  			if (elapsedTime > respawnTime){
	  			powerup.lastRemoveTime = std::nullopt;
  			}
  		}
  	},
  	.onMessage = [](Tags& tags, std::string& key, std::any& value) -> void {
  	},
	},

	TagUpdater {
		.attribute = "rail",
		.onAdd = [](Tags& tags, int32_t id, AttributeValue value) -> void {
	  	auto attrHandle = getAttrHandle(id);
	  	auto railData = getStrAttr(attrHandle, "data-pos");
	  	modassert(railData.has_value(), "no data for rail");

	  	auto railNamesRaw = getStrAttr(attrHandle, "data-name");
	  	modassert(railNamesRaw.has_value(), "no name for rails");
	  	auto railNames = split(railNamesRaw.value(), ',');
	  	auto railIndexsRaw = getStrAttr(attrHandle, "data-index");
	  	modassert(railIndexsRaw.has_value(), "no index for rails");
	  	std::vector<int> railIndexs;
	  	for(auto& railIndexStr : split(railIndexsRaw.value(), ',')){
	  		int railIndex = std::atoi(railIndexStr.c_str());
	  		railIndexs.push_back(railIndex);
	  	}

	  	auto posValuesStr = split(railData.value(), ',');	
	  	std::cout << "rail raw data: " << print(posValuesStr) << std::endl;
	  	std::cout << "rail names: " << print(railNames) << std::endl;
	  	std::cout << "rail indexs: " << print(railIndexs) << std::endl;

			std::vector<RailNode> nodes;
	  	for (int i = 0; i < posValuesStr.size(); i++){
  		  auto& posStr = posValuesStr.at(i);
  		  auto railName = railNames.at(i);
  		  auto railIndex = railIndexs.at(i);
	  		auto posVec = parseVec3(posStr);
	  		nodes.push_back(RailNode {
	  			.rail = railName,
	  			.railIndex = railIndex,
	  			.point = posVec,
	  		});
	  	}

	  	addRails(nodes);

	  	std::string targetEntity = "entity_dynamic_7";
	  	auto managedRailEntityId = gameapi -> getGameObjectByName(targetEntity, gameapi -> listSceneId(id)).value();
	  	auto railId = railIdForName("rail1");

	  	managedRailMovements[managedRailEntityId] = ManagedRailMovement {
	  		.railId = railId.value(),
	  		.initialObjectPos = gameapi -> getGameObjectPos(managedRailEntityId, true, "[gamelogic] - managed rail movement get init pos"),
	  	};


		},
  	.onRemove = [](Tags& tags, int32_t id) -> void {
  		rails.erase(id);
  	},
  	.onFrame = [](Tags& tags) -> void {
  	
  	},
  	.onMessage = [](Tags& tags, std::string& key, std::any& value) -> void {
  	},
	}
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
  tags.healthColorObjects = {};
  tags.teleportObjs = {};
  tags.explosionObjects = {};
  tags.linkGunObj = {};
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
	for(auto &[id, teleportExit] : tags.teleportObjs){
		if (currIndex == index){
			auto position = gameapi -> getGameObjectPos(id, true, "[gamelogic] tags - getTeleportPosition");
			return TeleportInfo {
				.id = id,
				.position = position
			};
		}
		currIndex++;
	}
	return std::nullopt;
}
