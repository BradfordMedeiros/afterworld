#include "./tags.h"

glm::quat quatFromTrenchBroomAngles(float pitch, float yaw, float roll);

extern CustomApiBindings* gameapi;

extern Waypoints waypoints;
extern ArcadeApi arcadeApi;
extern Director director;
extern Vehicles vehicles;
extern GameTypes gametypeSystem;
extern OrbData orbData;
extern std::optional<std::string> activeLevel;
extern std::unordered_map<objid, LinePoints> rails;
extern std::unordered_map<objid, ManagedRailMovement> managedRailMovements;
std::set<objid> managedRailMovementsBuffer; // because this can be added before the rail its referencing is available
extern std::unordered_map<objid, Laser> lasers;
extern std::unordered_map<objid, GravityWell> gravityWells;
extern std::unordered_map<objid, TriggerColor> triggerColors;
extern std::unordered_map<objid, TeleportExit> teleportObjs;
extern std::unordered_map<objid, Powerup> powerups;
extern StateController animationController;
extern std::set<objid> textureScrollObjIds;
extern AudioZones audiozones;
extern InGameUi inGameUi;
extern std::unordered_map<objid, SpinObject> idToRotateTimeAdded;
extern std::unordered_map<objid, EmissionObject> emissionObjects;
extern std::unordered_map<objid, HealthColorObject> healthColorObjects;
extern std::unordered_map<objid, ExplosionObj> explosionObjects;

void goToLevel(std::string levelShortName);
void goBackMainMenu();
glm::quat quatFromTrenchBroomAngles(float pitch, float yaw, float roll);

struct TagUpdater {
	std::string attribute;
	std::function<void(int32_t idAdded, AttributeValue value)> onAdd;
	std::function<void(int32_t idAdded)> onRemove;
	std::optional<std::function<void()>> onFrame;
	std::optional<std::function<void(std::string& key, std::any& value)>> onMessage;
};

bool isInGameMode2(){
	return getGlobalState().routeState.inGameMode;	
}


std::string queryInitialBackground(){
	if (hasOption("background")){
		return getArgOption("background");
	}
	return getSaveStringValue("settings", "background", "../gameresources/textures/backgrounds/test3.png");
}


std::vector<glm::vec3> parseDataVec3(std::string& value){
	std::vector<glm::vec3> values;
	auto posValuesStr = split(value, ',');	
	for (auto& posValueStr : posValuesStr){
		auto posVec = parseVec3(posValueStr);	  	
		values.push_back(posVec);
	}
	return values;
}

std::vector<std::string> parseDataString(std::string& value){
	return split(value, ',');
}

void onAddConditionId(objid id, std::string& value){
	auto gem = getSingleAttr(id, "gem-label");
	if (gem.has_value()){
		if (hasCrystal(gem.value())){
			gameapi -> removeByGroupId(id);
			return;
		}		
	}
}


struct Skippable {
	std::optional<std::string> advanceToLevel;
};
std::unordered_map<objid, Skippable> skippable;

std::vector<TagUpdater> tagupdates = { 
	TagUpdater {
		.attribute = "animation",  // TODO this should probably move to entity
		.onAdd = [](int32_t id, AttributeValue attrValue) -> void {
			auto value = maybeUnwrapAttrOpt<std::string>(attrValue).value();
  		auto animationControllerValue = getSingleAttr(id, "animation");
  		addEntityController(animationController, id, getSymbol(animationControllerValue.value()));
  	},
  	.onRemove = [](int32_t id) -> void {
		 	removeEntityController(animationController, id);
  	},
  	.onFrame = std::nullopt,
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "scrollspeed",
		.onAdd = [](int32_t id, AttributeValue attrValue) -> void {
			auto value = maybeUnwrapAttrOpt<glm::vec3>(attrValue);
			modassert(value.has_value(), "scrollspeed not vec3");
  		    //std::cout << "scroll: on object add: " << gameapi -> getGameObjNameForId(id).value() << std::endl;
  		    textureScrollObjIds.insert(id);
  	    },
  	    .onRemove = [](int32_t id) -> void {
 	    	textureScrollObjIds.erase(id);
  	    },
  	    .onFrame = []() -> void {
  	    	//std::cout << "scrollspeed: on frame: " << tags.textureScrollObjIds.size() <<  std::endl;
  	    	if (!isInGameMode2()){
  	    		return;
  	    	}
	    	handleScroll(textureScrollObjIds);
  	    },
  	    .onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "health",
		.onAdd = [](int32_t id, AttributeValue) -> void {
 			modlog("health", "entity added: " + std::to_string(id));
 			addEntityIdHitpoints(id);
  	     },
  	     .onRemove = [](int32_t id) -> void {
	        modlog("health", "entity removed: " + std::to_string(id));
	        removeEntityIdHitpoints(id);
  	     },
  	     .onFrame = std::nullopt,
  	     .onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "ambient",
		.onAdd = [](int32_t id, AttributeValue) -> void {
            addAudioZone(id);
  	    },
  	    .onRemove = [](int32_t id) -> void {
            removeAudioZone(id);
  	    },
  	    .onFrame = []() -> void {
            onAudioZoneFrame();
  	    },
  	    .onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "spawn",
		.onAdd = [](int32_t id, AttributeValue) -> void {
			spawnAddId(director.managedSpawnpoints, id);
		},
  	    .onRemove = [](int32_t id) -> void {
  	    	spawnRemoveId(director.managedSpawnpoints, id);
  	    },
  	    .onFrame = []() -> void {
  	    	if (!isInGameMode2()){
  	    		return;
  	    	}
	        onSpawnTick(director.managedSpawnpoints);
  	    },
  	    .onMessage = [](std::string& key, std::any& value) -> void {},
	},
	TagUpdater {
		.attribute = "spawn-managed",
		.onAdd = [](int32_t id, AttributeValue) -> void {
			modlog("spawn-manage added: ", gameapi -> getGameObjNameForId(id).value());
		},
  	    .onRemove = [](int32_t id) -> void {
  	    	spawnRemoveId(director.managedSpawnpoints, id);
  	    },
  	    .onFrame = []() -> void {},
  	    .onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "destroy",
		.onAdd = [](int32_t id, AttributeValue) -> void {},
  		.onRemove = [](int32_t id) -> void {
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
		.onAdd = [](int32_t id, AttributeValue) -> void {},
  		.onRemove = [](int32_t id) -> void {
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
		.onAdd = [](int32_t id, AttributeValue) -> void {
			// do not do it this way...
  			auto attrHandle = getAttrHandle(id);
			auto explodeAfterSeconds = getFloatAttr(attrHandle, "damageafter").value();
			explosionObjects[id] = ExplosionObj {
				.time = gameapi -> timeSeconds(false) + explodeAfterSeconds,
			};
		},
  	    .onRemove = [](int32_t id) -> void {
  	    	explosionObjects.erase(id);
  	    },
  	    .onFrame = []() -> void {
   	    	auto currTime = gameapi -> timeSeconds(false);
    
   	    	std::vector<objid> explodedObjs;
   	    	for (auto &[id, explosionObj] : explosionObjects){
  	    		if (currTime >= explosionObj.time){
  	    			explodedObjs.push_back(id);
	    				doDamageMessage(id, 100.f);
  	    		}
  	    	}
  	    	for (auto id : explodedObjs){
  	    		explosionObjects.erase(id);
  	    	}
  	    },
  	    .onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "linkorb", 
		.onAdd = [](int32_t id, AttributeValue) -> void {
			addLinkGunObj(id);
		},
  	    .onRemove = [](int32_t id) -> void {
  	    	removeLinkGunObj(id);
  	    },
  	    .onFrame = []() -> void {
  	    	onLinkGunObjFrame();
  	    },
  	    .onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "condition",
		.onAdd = [](int32_t id, AttributeValue attrValue) -> void {
 			auto value = maybeUnwrapAttrOpt<std::string>(attrValue).value();
			onAddConditionId(id, value);
		},
  	.onRemove = [](int32_t id) -> void {},
  	.onFrame = std::nullopt,
  	.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "in-game-ui",
		.onAdd = []( int32_t id, AttributeValue) -> void {
			createInGamesUiInstance(inGameUi, id);
		},
  		.onRemove = [](int32_t id) -> void {
  			freeInGameUiInstance(inGameUi, id);
  		},
  		.onFrame = []() -> void {},
  		.onMessage = [](std::string& key, std::any& value) -> void { 		  
  		},
	},
	TagUpdater {
		.attribute = "spin",
		.onAdd = [](int32_t id, AttributeValue) -> void {
            startRotate(id);
		},
  		.onRemove = [](int32_t id) -> void {
            stopRotate(id);
  		},
  		.onFrame = []() -> void {
  			onRotateFrame(isInGameMode2());
  		},
  		.onMessage =  std::nullopt,
	},
	TagUpdater {
		.attribute = "teleport",
		.onAdd = [](int32_t id, AttributeValue) -> void {
	  	    auto attrHandle = getAttrHandle(id);
			auto teleportExit = getStrAttr(attrHandle, "teleport_exit");
  	        teleportObjs[id] = TeleportExit{
  			   .exit = teleportExit,
  		    };
		},
  		.onRemove = [](int32_t id) -> void {
  			teleportObjs.erase(id);
  		},
  		.onFrame = std::nullopt,
  		.onMessage =  std::nullopt,
	},
	TagUpdater {
		.attribute = "autoplay",
		.onAdd = [](int32_t id, AttributeValue) -> void {
  			playMusicClipById(id, std::nullopt);
		},
  		.onRemove = [](int32_t id) -> void {},
  		.onFrame = std::nullopt,
  		.onMessage =  std::nullopt,
	},
	TagUpdater {
		.attribute = "background",
		.onAdd = [](int32_t id, AttributeValue) -> void {
	  		setGameObjectTexture(id, queryInitialBackground());
		},
  		.onRemove = [](int32_t id) -> void {},
  		.onFrame = std::nullopt,
  		.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "glasstexture",
		.onAdd = [](int32_t id, AttributeValue) -> void {
	  		createGlassTexture(id);
		},
  		.onRemove = [](int32_t id) -> void {
  			removeGlassTexture(id);
  		},
  		.onFrame = std::nullopt,
  		.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "waypoint",
		.onAdd = [](int32_t id, AttributeValue) -> void {
	  	    addWaypoint(waypoints, id, id);
	  	    auto hitpoints = getHealth(id);
	  	    if (hitpoints.has_value()){
	  	    	updateHealth(waypoints, id, hitpoints.value().current / hitpoints.value().total);
	  	    }
		},
  		.onRemove = [](int32_t id) -> void {
  			removeWaypoint(waypoints, id);
  		},
  		.onFrame = std::nullopt,
  		.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "autoemission",
		.onAdd = [](int32_t id, AttributeValue value) -> void {
			auto period = maybeUnwrapAttrOpt<float>(value).value();
	  		auto attrHandle = getAttrHandle(id);
			auto emissionLow = getVec3Attr(attrHandle, "autoemission-low");
			auto emissionHigh = getVec3Attr(attrHandle, "autoemission-high");

            addEmissionObj(
                id,
                emissionLow.has_value() ? emissionLow.value(): glm::vec3(0.f, 0.f, 0.f),
                emissionHigh.has_value() ? emissionHigh.value() : glm::vec3(1.f, 1.f, 1.f),
                period
            );
		},
  		.onRemove = [](int32_t id) -> void {
            removeEmissionObj(id);
  		},
  		.onFrame = []() -> void {
            onEmissionFrame();
  		},
  		.onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "healthcolor",
		.onAdd = [](int32_t id, AttributeValue value) -> void {
	  		auto attrHandle = getAttrHandle(id);
			auto colorLow = getVec3Attr(attrHandle, "healthcolor-low");
			auto colorHigh = getVec3Attr(attrHandle, "healthcolor");
			auto colorTarget = getStrAttr(attrHandle, "healthcolor-target");
			auto target = colorTarget.has_value() ? findBodyPart(id, colorTarget.value().c_str()) : std::nullopt;

	  		healthColorObjects[id] = HealthColorObject {
	  			.lowColor = colorLow.has_value() ? colorLow.value(): glm::vec3(0.f, 0.f, 0.f),
	  			.highColor = colorHigh.has_value() ? colorHigh.value() : glm::vec3(1.f, 1.f, 1.f),
	  			.target = target,
	  		};
		},
  	    .onRemove = [](int32_t id) -> void {
  	    	healthColorObjects.erase(id);
  	    },
  	    .onFrame = []() -> void {
  	    	if (!isInGameMode2()){
  	    		return;
  	    	}
  	    	for (auto &[id, healthColorObject] : healthColorObjects){
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
		.onAdd = [](int32_t id, AttributeValue value) -> void {
			auto cutscene = getSingleAttr(id, "cutscene");
			playCutsceneScript(id, cutscene.value());
		},
  	    .onRemove = [](int32_t id) -> void {
	        removeCutscene(id);
  	    },
  	    .onFrame = std::nullopt,
  	    .onMessage = std::nullopt,
	},
	TagUpdater {
		.attribute = "arcade",
		.onAdd = [](int32_t id, AttributeValue value) -> void {
			modlog("arcade", "on add");
			std::string textureName = std::string("arcade-texture") + std::to_string(id);
			auto arcadeTextureId = gameapi -> createTexture(textureName, 1000, 1000, id);
            gameapi -> drawRect(0.f /*centerX*/, 0.f /*centerY*/, 2.f, 2.f, false, glm::vec4(1.f, 0.f, 1.f, 0.75f), arcadeTextureId, true, std::nullopt, "./res/textures/water.jpg", std::nullopt);
		 	setGameObjectTexture(id, textureName);

  		    auto arcadeType = getSingleAttr(id, "arcade");
            addArcadeType(id, arcadeType.value(), arcadeTextureId);
		},
  	     .onRemove = [](int32_t id) -> void {
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
		.attribute = "globallight",
		.onAdd = [](int32_t id, AttributeValue value) -> void {
			gameapi -> setGlobalLight(id);
		},
  	    .onRemove = [](int32_t id) -> void {
  	    },
  	    .onFrame = std::nullopt,
  	    .onMessage = [](std::string& key, std::any& value) -> void {
  	    },
	},
	TagUpdater {
		.attribute = "orbui",
		.onAdd = [](int32_t id, AttributeValue value) -> void {
	  	auto attrHandle = getAttrHandle(id);
	  	auto orbPositionStrs = getStrAttr(attrHandle, "data-pos");
	  	modassert(orbPositionStrs.has_value(), "no data for orbs");
			auto orbPositions = parseDataVec3(orbPositionStrs.value());
			std::cout << "orbui: ";
			for (auto pos : orbPositions){
				std::cout << print(pos) << " ";
			}
			std::cout << std::endl;

			auto orbLevelsStr = getStrAttr(attrHandle, "data-level");
			modassert(orbLevelsStr.has_value(), "no data for orb levels");
			auto orbLevels = split(orbLevelsStr.value(), ',');

			auto orbNamesStr = getStrAttr(attrHandle, "data-name");
	  	modassert(orbNamesStr.has_value(), "no data-name for orbs");
	  	std::set<std::string> names;

	  	auto orbNames = parseDataString(orbNamesStr.value());

			auto orbUisStr = getStrAttr(attrHandle, "data-orbui");
	  	modassert(orbUisStr.has_value(), "no data-orbui for orbs");
	  	auto orbUisData = parseDataString(orbUisStr.value());

	  	auto railDataRot = getStrAttr(attrHandle, "data-rot");
	  	modassert(railDataRot.has_value(), "no data for rail-rot");
	  	auto dataRotations = parseDataVec3(railDataRot.value());

			std::vector<OrbDataConfig> orbDatas;
			for (int i = 0; i < orbPositions.size(); i++){
				auto rotVec = dataRotations.at(i);
				orbDatas.push_back(OrbDataConfig {
					.pos = orbPositions.at(i),
					.rotation = quatFromTrenchBroomAngles(rotVec.x, rotVec.y, rotVec.z),
					.level = orbLevels.at(i),
					.orbUi = orbUisData.at(i),
				});
			}

			auto orbConnStr = getStrAttr(attrHandle, "data-conn");
			modassert(orbConnStr.has_value(), "no data-conn for orbs");
			auto orbConnStrs = split(orbConnStr.value(), ',');

			std::vector<OrbDataConection> orbConns;
			for (int i = 0; i < orbConnStrs.size(); i++){
				auto toConns = split(orbConnStrs.at(i), '-');
				for (auto& conn : toConns){
					auto toIndex = std::atoi(conn.c_str());
					orbConns.push_back(OrbDataConection {
							.connection = OrbConnection {
								.indexFrom = i,
								.indexTo = toIndex,
							},
							.orbUi = orbUisData.at(i),
					});
				}
			}

			auto orbUis = createOrbUi2(id, orbDatas, orbConns);
			for (auto& orbUi : orbUis){
				orbData.orbUis[getUniqueObjId()] = orbUi;			
			}
		},
  	    .onRemove = [](int32_t id) -> void {
  	    	std::set<objid> idsToRemove;
  	    	for (auto&[id, orbUi] : orbData.orbUis){
  	    		if (orbUi.ownerId == id){
  	    			idsToRemove.insert(id);
  	    		}
  	    	}
  	    	for (auto id : idsToRemove){
	        		orbData.orbUis.erase(id);
  	    	}
  	    },
  	    .onFrame = []() -> void {
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
  	    .onMessage = [](std::string& key, std::any& value) -> void {},
	},
	TagUpdater {
		.attribute = "vehicle",
		.onAdd = [](int32_t id, AttributeValue value) -> void {
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
  	    .onRemove = [](int32_t id) -> void {
  	    	removeVehicle(vehicles, id);
  	    },
  	    .onFrame = std::nullopt,
  	    .onMessage = [](std::string& key, std::any& value) -> void {},
	},
	// Is this duplicate with the end level thing?
	TagUpdater {
		.attribute = "advance",
		.onAdd = [](int32_t id, AttributeValue value) -> void {
 			auto advanceToLevel = getSingleAttr(id, "advance");
			skippable[id] = Skippable{
				.advanceToLevel = advanceToLevel,
			};
		},
  	    .onRemove = [](int32_t id) -> void {
 	    		auto levelComplete = getSingleAttr(id, "markcomplete");
 	    		if (levelComplete.has_value()){
 	    			markLevelComplete(levelComplete.value(), 0.f);
 	    		}
 	    		skippable.erase(id);
  	    },
  	    .onFrame = []() -> void {},
  	    .onMessage = [](std::string& key, std::any& value) -> void {
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
		.attribute = "powerup",
		.onAdd = [](int32_t id, AttributeValue value) -> void {
			auto attrHandle = getAttrHandle(id);
	    	auto powerup = getSingleAttr(id, "powerup");
	    	auto respawnRate = getSingleFloatAttr(id, "powerup-rate");
	    	auto tint = getVec4Attr(attrHandle, "tint");

	    	std::optional<int> respawnRateMs;
	    	if (respawnRate.has_value()){
	    		respawnRateMs = static_cast<int>(respawnRate.value());   
	    	}  

			powerups[id] = Powerup {
				.type = powerup.value(),
				.tint = tint.has_value() ? tint.value() : glm::vec4(1.f, 1.f, 1.f, 1.f),
				.respawnRateMs = respawnRateMs,
				.disabledVisually = false,
			};
		},
  		.onRemove = [](int32_t id) -> void {
  			powerups.erase(id);
  		},
  	    .onFrame = []() -> void {
  	    	float currTime = gameapi -> timeSeconds(false);
  	    	for (auto& [id, powerup] : powerups){
  	    		if (powerup.lastRemoveTime.has_value() && !powerup.disabledVisually){
        			setGameObjectTint(id, glm::vec4(powerup.tint.x, powerup.tint.y, powerup.tint.z, 0.2f * powerup.tint.w));
        			powerup.disabledVisually = true;
  	    		}
  	    		if (!powerup.lastRemoveTime.has_value() && powerup.disabledVisually){
        			setGameObjectTint(id, powerup.tint);
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
  		.onMessage = [](std::string& key, std::any& value) -> void {},
	},
	TagUpdater {
		.attribute = "rail",
		.onAdd = [](int32_t id, AttributeValue value) -> void {
	  	auto attrHandle = getAttrHandle(id);

	  	auto railData = getStrAttr(attrHandle, "data-pos");
	  	modassert(railData.has_value(), "no data for rail");
			auto dataPositions = parseDataVec3(railData.value());
	
	  	auto railDataRot = getStrAttr(attrHandle, "data-rot");
	  	modassert(railDataRot.has_value(), "no data for rail-rot");
	  	auto dataRotations = parseDataVec3(railDataRot.value());

	  	auto railNamesRaw = getStrAttr(attrHandle, "data-name");
	  	modassert(railNamesRaw.has_value(), "no name for rails");
	  	auto railNames = parseDataString(railNamesRaw.value());

	  	auto railIndexsRaw = getStrAttr(attrHandle, "data-index");
	  	modassert(railIndexsRaw.has_value(), "no index for rails");
	  	std::vector<int> railIndexs;
	  	for(auto& railIndexStr : split(railIndexsRaw.value(), ',')){
	  		int railIndex = std::atoi(railIndexStr.c_str());
	  		railIndexs.push_back(railIndex);
	  	}

	  	auto railTimesRaw = getStrAttr(attrHandle, "data-time");
	  	modassert(railTimesRaw.has_value(), "no time for rails");
	  	std::vector<int> railTimes;
	  	for(auto& railTimeStr : split(railTimesRaw.value(), ',')){
	  		int railTime = std::atoi(railTimeStr.c_str());
	  		railTimes.push_back(railTime);
	  	}

	  	auto railVisualizeRaw = getStrAttr(attrHandle, "data-visual");
	  	modassert(railVisualizeRaw.has_value(), "no visual for rails");
	  	auto railVisualizations = parseDataString(railVisualizeRaw.value());

			std::vector<RailNode> nodes;
	  	for (int i = 0; i < dataPositions.size(); i++){
  		  auto railName = railNames.at(i);
  		  auto railIndex = railIndexs.at(i);
  		  auto railTime = railTimes.at(i);
	  		auto posVec = dataPositions.at(i);
	  		auto rotVec = dataRotations.at(i);
	  		auto railVisual = railVisualizations.at(i);
	  		nodes.push_back(RailNode {
	  			.rail = railName,
	  			.railIndex = railIndex,
	  			.point = posVec,
	  			.rotation = quatFromTrenchBroomAngles(rotVec.x, rotVec.y, rotVec.z),
	  			.time = railTime,
	  			.visual = railVisual != "" ? railVisual : std::optional<std::string>(std::nullopt),
	  		});
	  	}

	  	addRails(id, nodes);
		},
  	    .onRemove = [](int32_t id) -> void {
  	    	removeRails(id);
  	    },
  	    .onFrame = []() -> void {
  	    
  	    },
  	    .onMessage = [](std::string& key, std::any& value) -> void {
  	    },
	},
	TagUpdater {
		.attribute = "curve",
		.onAdd = [](int32_t id, AttributeValue value) -> void {
			managedRailMovementsBuffer.insert(id);
		},
  		.onRemove = [](int32_t id) -> void {
  			managedRailMovementsBuffer.erase(id);
  			removeManagedRailMovement(id);
  		},
  		.onFrame = []() -> void {
  			auto bufferToAdd = managedRailMovementsBuffer;
  			for (auto id : bufferToAdd){
  	  			auto attrHandle = getAttrHandle(id);
				auto curve = getStrAttr(attrHandle, "curve").value();
				auto trigger = getStrAttr(attrHandle, "trigger");
				auto railId = railIdForName(curve);
				if (railId.has_value()){
					managedRailMovementsBuffer.erase(id);
					managedRailMovements[id] = ManagedRailMovement {
						.railId = railId.value(),
						.initialObjectPos = gameapi -> getGameObjectPos(id, true, "[gamelogic] - managed rail movement get init pos"),
						.initialObjectRot = gameapi -> getGameObjectRotation(id, true, "[gamelogic] - managed rail movement get init rot"),
						.autostart = false,
						.initialStartTime = std::nullopt,
						.trigger = trigger,
					};
				}
  			}
  		},
  		.onMessage = [](std::string& key, std::any& value) -> void {},
	},
	TagUpdater {
		.attribute = "laser",
		.onAdd = [](int32_t id, AttributeValue value) -> void {
			auto attrHandle = getAttrHandle(id);
			auto laserLength = getFloatAttr(attrHandle, "laserlength");
			addLaser(id, laserLength.has_value() ? laserLength.value() : 5.f);
		},
  	    .onRemove = [](int32_t id) -> void {
  	    	removeLaser(id);
  	    },
  	    .onFrame = []() -> void {
  	    	onLaserFrame();
  	    },
  	    .onMessage = [](std::string& key, std::any& value) -> void {},
	},
	TagUpdater {
		.attribute = "gravityhole",
		.onAdd = [](int32_t id, AttributeValue value) -> void {
			auto attrHandle = getAttrHandle(id);
			auto wellName = getStrAttr(attrHandle, "wellname");
			auto targetWell = getStrAttr(attrHandle, "targetwell");
			auto launcherAmount = getVec3Attr(attrHandle, "launch");
			auto autolaunch = getStrAttr(attrHandle, "holemode");

			gravityWells[id] = GravityWell{
				.id = id,
				.name = wellName,
				.target = targetWell,
				.autolaunch = autolaunch.has_value() ? (autolaunch.value() == "auto") : false,
				.launcher = launcherAmount,
			};
		},
  	    .onRemove = [](int32_t id) -> void {
  	    	gravityWells.erase(id);
  	    },
  	    .onFrame = []() -> void {
  	    	onFrameGravityWells();
  	    },
  	    .onMessage = [](std::string& key, std::any& value) -> void {},
	},
	TagUpdater {
		.attribute = "triggercolor",
		.onAdd = [](int32_t id, AttributeValue value) -> void {
		  auto objHandle = getAttrHandle(id);
			auto activeColor = getVec4Attr(objHandle, "activecolor");
			auto unactiveColor = getVec4Attr(objHandle, "unactivecolor");

			auto trigger = getStrAttr(objHandle, "triggercolor").value();
			triggerColors[id] = TriggerColor {
				.trigger = trigger,
				.activeColor = activeColor.has_value() ? activeColor.value() : std::optional<glm::vec4>(std::nullopt),
				.unactiveColor = unactiveColor.has_value() ? unactiveColor.value() : std::optional<glm::vec4>(std::nullopt),
			};
			if (triggerColors.at(id).unactiveColor.has_value()){
				setGameObjectTint(id, triggerColors.at(id).unactiveColor.value());
			}
		},
  	    .onRemove = [](int32_t id) -> void {
  	    	triggerColors.erase(id);
  	    },
  	    .onFrame = []() -> void {
  	    },
  	    .onMessage = [](std::string& key, std::any& value) -> void {},
	},

};

void onTagsMessage(std::string& key, std::any& value){
  for (auto &tagUpdate : tagupdates){
  	if (tagUpdate.onMessage.has_value()){
  		tagUpdate.onMessage.value()(key, value);
  	}
  }
}
void onTagsFrame(){
	for (auto &tagUpdate : tagupdates){
		if (tagUpdate.onFrame.has_value()){
			tagUpdate.onFrame.value()();
		}
	}
}

void handleOnAddedTags(int32_t idAdded){
  auto objHandle = getAttrHandle(idAdded);
	for (auto &tagUpdate : tagupdates){
    auto attrValue = getAttr(objHandle, tagUpdate.attribute.c_str());
    if (attrValue.has_value()){
	    tagUpdate.onAdd(idAdded, attrValue.value());
    }
	}
}
void handleOnAddedTagsInitial(){
  std::set<objid> idsAlreadyExisting;
  for (auto &tagUpdate : tagupdates){
  	auto ids = gameapi -> getObjectsByAttr(tagUpdate.attribute, std::nullopt, std::nullopt);
  	for (auto idAdded : ids){
	  	idsAlreadyExisting.insert(idAdded);
  	}
  }
  for (auto idAdded : idsAlreadyExisting){
  	handleOnAddedTags(idAdded);
  }
}


void handleTagsOnObjectRemoved(int32_t idRemoved){
 	for (auto &tagUpdate : tagupdates){
	    if (hasAttribute(idRemoved, tagUpdate.attribute.c_str())){
        tagUpdate.onRemove(idRemoved);
      }
	}
}
