#include "./ball.h"

extern CustomApiBindings* gameapi;
extern GLFWwindow* window;

extern GameTypes gametypeSystem;
extern std::optional<std::string> activeLevel;
extern Vehicles vehicles;
extern Movement movement;
extern Weapons weapons;
extern std::unordered_map<objid, Powerup> powerups;
extern std::unordered_map<objid, Activatable> activateables;

void goToLevel(std::string levelShortName);
void setPauseMenuOverride(std::optional<std::function<void()>> goToMenuFn);
void inputOverride(bool paused, bool showMouse);
void inputOverride();
bool isReloadKey(int button);
void handleRemoveKillplaneCollision(objid);

struct WorldView {
	bool didChangeToOrb = false;
	bool onMultiview = false;
	std::string world;
};

enum BallModeSpirit {
	MODE_NONE,
	MODE_RED,
	MODE_BLUE,
	MODE_YELLOW,
	MODE_PURPLE,
};

struct BallModeOptions{
   std::optional<glm::vec3> initialBallPos;
   objid ballId;
   objid spiritId;
   objid eyeId;
   objid sceneId;
   bool didLose = false;
   bool shouldReset = false;
   bool didReset = false;
   std::optional<float> ballStartTime;
   std::optional<float> finalBallTime;
   BallModeSpirit spirit = MODE_NONE;
   std::optional<BallModeSpirit> changeSpirit;

   std::optional<WorldView> worldView;

   bool levelSelect = false;

   std::optional<objid> rebirthSphere;
   std::optional<glm::vec3> rebirthSphereInitialPos;

   glm::vec3 playerSpawnPosition;
};

BallModeOptions& getBallModeOptions(){
	auto data = getGametypeData(gametypeSystem);
	modassert(data.has_value(), "getBallMode options - gametype system empty data");
 	BallModeOptions* ballModePtr = std::any_cast<BallModeOptions>(data.value());
 	modassert(ballModePtr, "no active ball mode");
 	return *ballModePtr;
}

bool inBallGametype(){
	auto data = getGametypeData(gametypeSystem);
	return data.has_value();
}

struct DescInfo2 {
	std::vector<std::string> mainInfos;
	std::vector<std::string> hubInfos;
	std::vector<std::string> levelInfos;
	std::vector<WorldOrbInfos> worldOrbInfos;
	bool onOverworld;
};

DescInfo2 getDescriptionInfo2(MultiOrbView& multiOrbView){
	bool onOverworld = isOverworld(multiOrbView);

	std::string worldName = "test";  // multiOrbView.activeWorldName
	std::optional<std::string> levelName = getSelectedLevel(multiOrbView);


	auto progressInfo = getPlaylistProgressInfo();
	auto worldProgressInfo = getWorldProgressInfo(worldName);
	DescInfo2 descInfo {
		.mainInfos = {
			std::string("overworld: ") + (onOverworld ? "true" : "false"),
			std::string("total gems: ") + std::to_string(progressInfo.gemCount) + " / " + std::to_string(progressInfo.totalGemCount),
		},
		.hubInfos = {
			std::string("world: ") + worldProgressInfo.currentWorld,
			std::string("completed: ") + std::to_string(progressInfo.completedLevels) + " / " + std::to_string(progressInfo.totalLevels),
			std::string("total gems: ") + std::to_string(worldProgressInfo.gemCount) + " / " + std::to_string(worldProgressInfo.totalGemCount),
		},
		.levelInfos = {},
		.onOverworld = onOverworld,
	};

	if (levelName.has_value()){
		auto levelProgressInfo = getLevelProgressInfo(worldName, levelName.value());
 		std::string parTime = print(levelProgressInfo.parTime, 2);
 		std::string bestTime = "n/a";
 		if(levelProgressInfo.bestTime.has_value()){
 			bestTime = print(levelProgressInfo.bestTime.value(), 2);
 		}
		descInfo.levelInfos = {
			std::string("current level: ") + levelName.value(),
			std::string("par time: ") + parTime + "s",
			std::string("best time: ") + bestTime + "s",
			std::string("total gems: ") + std::to_string(levelProgressInfo.gemCount) + " / " + std::to_string(levelProgressInfo.totalGemCount),
		};
	}

	descInfo.worldOrbInfos = getOrbUiData(multiOrbView);

	return descInfo;
}

void drawDescInfo2(DescInfo2& descInfo){
	for (int i = 0; i < descInfo.worldOrbInfos.size(); i++){
		 auto& info = descInfo.worldOrbInfos.at(i);
     gameapi -> drawRect(0.95f, 0.75 + (-0.1 * i), 0.05f, 0.05f, false, glm::vec4(0.f, 0.f, 0.f, 0.8f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);	  		
     if (info.isComplete){
	 	   gameapi -> drawRect(0.95f, 0.75 + (-0.1 * i), 0.01f, 0.01f, false, glm::vec4(1.0f, 0.9216f, 0.2314f, 0.4f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);	  		
     }
     if (info.selected){
     	gameapi -> drawRect(0.95f + (0.05f * 0.5f), 0.75 + (-0.1 * i), 0.005f, 0.05f, false, glm::vec4(0.f, 0.f, 1.f, 0.9f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);	  		
     }
	}

 	for (int i = 0; i < descInfo.hubInfos.size(); i++){
		gameapi -> drawText(descInfo.hubInfos.at(i), -0.9f, 0.7f - (i * 0.1f), 12, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	}
	for (int i = 0; i < descInfo.mainInfos.size(); i++){
	 	gameapi -> drawText(descInfo.mainInfos.at(i), -0.9f, -0.6f - (i * 0.1f), 12, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	}
	if (!descInfo.onOverworld){
	 	for (int i = 0; i < descInfo.levelInfos.size(); i++){
	 		gameapi -> drawText(descInfo.levelInfos.at(i), 0.6f, 0.7f - (i * 0.1f), 8, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	 	}	  		
	}
}

objid ensureTempCamera(objid sceneId){
	std::string cameraName = ">tempcamera-ball";
  auto cameraId = findObjByShortName(cameraName, sceneId);
  if (cameraId.has_value()){
  	return cameraId.value();
  }

  GameobjAttributes attr {.attr = {} };
	std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
 	auto camera = gameapi -> makeObjectAttr(sceneId, cameraName, attr, submodelAttributes);  
 	modassert(camera.has_value(), "could not create tempcamera");
 	return camera.value();
}


void ballStartGameplay(EasyCutscene& cutscene){
	struct BallStartData {
		bool cutsceneFinished = false;
	};

  if (initialize(cutscene)){
    setEntityControlDisabled(true, getEntityForPlayerIndex(0).value());
    BallStartData startData {};
    store(cutscene, startData);
  }
  if (finalize(cutscene)){
  }

  run(cutscene, 2, [&cutscene]() -> void {
  		auto sceneId = gameapi -> listSceneId(getEntityForPlayerIndex(0).value());
 		  auto railId = railIdForName("levelintro", 0);

  		if (!railId.has_value()){
		  	BallStartData* startData = getStorage<BallStartData>(cutscene);
		  	modassert(startData, "ballStartGameplay startData is null");
		  	startData -> cutsceneFinished = true;
   			setTempCamera(std::nullopt, 0);
   			return;
  		}

  		setTempCamera(ensureTempCamera(sceneId), 0);
  		
  		std::vector<SimpleNarration> narrations;

  		
  		auto& railPoints = *railForId(railId.value()).value();
  		for (auto& key : railPoints.keys){
  			narrations.push_back(SimpleNarration {
  				.text = key.has_value() ? key.value() : "",
  			});
  		}
  		

  		NarratedMovement narratedMovement {
  			.railId = railId.value(),
  			.letterbox = "Welcome to The World",
  			.narrations = narrations,
  		};

  		bool skipCutscene = false;
  		if (skipCutscene){
		  	BallStartData* startData = getStorage<BallStartData>(cutscene);
		  	modassert(startData, "ballStartGameplay startData is null");
		  	startData -> cutsceneFinished = true;
   			setTempCamera(std::nullopt, 0);
  		}else{
 				playCutscene(simpleNarratedMovement(ensureTempCamera(sceneId), narratedMovement, false, [&cutscene]() -> void { 
		  		BallStartData* startData = getStorage<BallStartData>(cutscene);
		  		modassert(startData, "ballStartGameplay startData is null");
		  		startData -> cutsceneFinished = true;
   				setTempCamera(std::nullopt, 0);
				}), std::nullopt);
  		}
  });


	waitFor(cutscene, 3, [&cutscene]() -> bool {
	  BallStartData* startData = getStorage<BallStartData>(cutscene);
		modassert(startData, "ballStartGameplay startData is null");
		return startData -> cutsceneFinished;
	});


  run(cutscene, 4, []() -> void {
    setEntityControlDisabled(false, getEntityForPlayerIndex(0).value());
  });


}
void ballEndGameplay(EasyCutscene& cutscene){
	if (initialize(cutscene)){
	  playGameplayClipById(getManagedSounds().teleportObjId.value(), std::nullopt, std::nullopt, false);
    setEntityControlDisabled(true, getEntityForPlayerIndex(0).value());
	}
  waitUntil(cutscene, 0, 2000);

  if (finishedThisFrame(cutscene, 0)){
  	getBallModeUI().value() -> ballMode.levelComplete = BallLevelComplete{};
  }

	waitFor(cutscene, 1, []() -> bool {
		return getGlobalState().control.leftMouseDown;
	});

  run(cutscene, 2, []() -> void {
  	goToLevel("ballselect");
  });
}

void setBallLevelComplete(){
	std::cout << "set ball level complete: " << activeLevel.value() << std::endl;

	auto& ballModeOptions = getBallModeOptions();
	if (ballModeOptions.finalBallTime.has_value()){
		return;
	}
	auto timeElapsed = gameapi -> timeSeconds(false) - ballModeOptions.ballStartTime.value();
	ballModeOptions.finalBallTime = timeElapsed;

	markLevelComplete(activeLevel.value(), timeElapsed);
	commitCrystals();
	playCutscene(ballEndGameplay, std::nullopt);	
}

struct BallObj {
	objid id;
	objid eyeId;
	std::optional<objid> spiritId;
};
BallObj createBallObj(objid sceneId, glm::vec3 position){
  GameobjAttributes attr { 
  	.attr = {
  		{ "vehicle", "ball" },
			{ "texture", "./res/textures/wood.jpg" },
			{ "tint", glm::vec4(1.f, 1.f, 1.f, 1.f) },
			{ "physics_restitution", 0.5f },
			{ "mesh", "../gameresources/build/characters/ball.gltf" },
			{ "physics_mass" , 10.f },
			{ "physics_type", "dynamic" },
			{ "physics_shape", "shape_sphere" },
			{ "physics", "enabled" },
			{ "position", position},
			{ "layer", "transparency" },

  	} 
  };

	std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  submodelAttributes["ball1/eye"] = GameobjAttributes {
  	.attr = {
  		//{ "flipbook", glm::vec3(4.f, 4.f, 4.f) },
  		// { "tint", glm::vec4(0.f, 0.f, 5.f, 1.f) },
  	}
  };
 	auto ball = gameapi -> makeObjectAttr(sceneId, std::string("ball1"), attr, submodelAttributes);  

	auto eyeId = findChildObjBySuffix(ball.value(), "eye");
	modassert(eyeId.has_value(), "create ball no eye id");


  bool addParticle = true;
  std::optional<objid> spiritId;

  if (addParticle){
  
  	{
  		modassert(ball.has_value(), "ball was not created");
  		GameobjAttributes emitterAttr { 
  			.attr = {
 					{ "effekseer", "./res/particles/spirit-white.efkefc" },
 					{ "state", "enabled" },
  			} 
  		};
  		std::unordered_map<std::string, GameobjAttributes> submodelAttributesEmitter;
  		auto ballSpirit = gameapi -> makeObjectAttr(sceneId, std::string("+ballSpirit"), emitterAttr, submodelAttributesEmitter);
  		modassert(ballSpirit.has_value(), "ballSpirit was not created");
  		spiritId = ballSpirit;
  		gameapi -> makeParent(ballSpirit.value(), ball.value());
		}  	
  }


  {
  	GameobjAttributes emitterAttr { 
  		.attr = {
  			{ "clip", "../gameresources/sound/rolling.ogg" },
 				{ "loop", "true" },
  		} 
  	};
  	std::unordered_map<std::string, GameobjAttributes> submodelAttributesEmitter;
  	auto ballSound = gameapi -> makeObjectAttr(sceneId, std::string("&ballsoundmove"), emitterAttr, submodelAttributesEmitter);
  	modassert(ballSound.has_value(), "ballSound was not created");

  	gameapi -> makeParent(ballSound.value(), ball.value());
	}

	return BallObj {
		.id = ball.value(),
		.eyeId = eyeId.value(),
		.spiritId = spiritId,
	};
}

void setPowerupTexture(std::string texture, std::optional<float> startTime, std::optional<float> duration){
	getBallModeUI().value() -> ballMode.powerupTexture = texture;
	getBallModeUI().value() -> ballMode.powerupStartTime = startTime;
	getBallModeUI().value() -> ballMode.powerupDuration = duration;
}

std::optional<BallPowerupState> getPowerup(){
	auto ballId = getBallModeOptions().ballId;
	auto vehicleBall = getVehicleBall(vehicles, ballId);
	modassert(vehicleBall.has_value(), "get powerup - not a ball");
	return getBallPowerup(*vehicleBall.value());
} 


GameTypeInfo getBallMode();


void ballModeSetPlayMode(objid sceneId){
	setTempCamera(std::nullopt, 0);

	inputOverride();

  changeUiMode(BallModeUi{});

	setPauseMenuOverride([]() -> void {
    goToLevel("ballselect");
	});
  

	auto playerSpawnId = findObjByShortName("playerspawn", std::nullopt);
	modassert(playerSpawnId.has_value(), "ballModeSetPlayMode - cannot find playerspawn");
	auto playerSpawnPosition = gameapi -> getGameObjectPos(playerSpawnId.value(), true, "[gamelogic] ball - get playerspawn position");

  // TODO - no reason to actually create the prefab here
  auto prefabId = createPrefab(sceneId, "../afterworld/scenes/prefabs/enemy/player-cheap.rawscene",  playerSpawnPosition, {});    
  auto playerId = findObjByShortName("maincamera", sceneId);
  modassert(playerId.has_value(), "onSceneRouteChange, no playerId in scene to load");
  setEntityForPlayerIndex(playerId.value(), 0);
  setEntityActivateMask(playerId.value(), 0b11111111);
  //////////

  auto ballData = createBallObj(sceneId, playerSpawnPosition);
	auto ballId = ballData.id;


	GameTypeInfo ballMode = getBallMode();
	BallModeOptions modeOptions {};
	modeOptions.ballId = ballId;
	modeOptions.eyeId =  ballData.eyeId;
	modeOptions.spiritId = ballData.spiritId.value();
	modeOptions.ballStartTime = gameapi -> timeSeconds(false);
	modeOptions.sceneId = sceneId;
	modeOptions.playerSpawnPosition = playerSpawnPosition;

	//auto sphereObj = createObject(sceneId, "../gameresources/build/primitives/sphere.gltf", glm::vec3(100.f, 100.f, 100.f), glm::vec3(8.f, 8.f, 8.f));

	auto rebirthObj = gameapi -> getObjectsByAttr("rebirth", std::nullopt, sceneId);
	modassert(rebirthObj.size() == 1, "no rebirth object in this scene");

	modeOptions.rebirthSphere = rebirthObj.at(0);
	auto rebirthSphereInitialPos = gameapi -> getGameObjectPos(modeOptions.rebirthSphere.value(), true, "[gamelogic] rebirthSphereInitialPos");
	modeOptions.rebirthSphereInitialPos = rebirthSphereInitialPos;

	setGameObjectEmitterEffectTint(modeOptions.spiritId, glm::vec4(0.f, 1.f, 0.f, 0.f));

	changeGameType(gametypeSystem, ballMode, "ball", &modeOptions);
}

void ballModeNewGame2(objid sceneId){
  resetProgress();
  playGameplayClipById(getManagedSounds().teleportObjId.value(), std::nullopt, std::nullopt, false);

  inputOverride(false, false);
	changeUiMode(UiModeNone{});

  auto railId = railIdForName("intro", 0);
  NarratedMovement narratedMovement {
  	.railId = railId.value(),
  	.letterbox = "Welcome to The World",
  	.narrations = {
  		SimpleNarration {
  			.text = "this is the first part",
  		},
  		SimpleNarration {
  			.text = "",
  		},
  		SimpleNarration {
  			.text = "this is the third part",
  		},
  	},
  };

  auto cameraId = ensureTempCamera(sceneId);
	setTempCamera(cameraId, 0);

	bool skipCutscene = false;
	if (skipCutscene){
  			ballModeSetPlayMode(sceneId); 
	}else{
  	auto currentCutscene = playCutscene(simpleNarratedMovement(cameraId, narratedMovement, false, [sceneId]() -> void { 
  			ballModeSetPlayMode(sceneId); 
  	}), std::nullopt);		
	}
}



void startBallIntroMode(objid sceneId){	
	setPauseMenuOverride([]() -> void {
    goToLevel("ballselect");
	});

	//if (currentCutscene.has_value()){
	//	removeCutscene(currentCutscene.value(), true);
	//	currentCutscene = std::nullopt;
	//}
  auto cameraId = findObjByShortName(">menu-view", sceneId);
  if (cameraId.has_value()){
		removeCameraFromMultiOrbView(cameraId.value());
	  setTempCamera(cameraId.value(), 0);  	
  }


  inputOverride(false, true);
  changeUiMode(LiveMenu {
   	.options = MainMenu2Options {
   		.backgroundColor = glm::vec4(1.f, 0.f, 0.f, 1.f),
   		.offsetY = 0.f,
			.onNewGame = [sceneId]() -> void {
				hideLetterBox();
				ballModeNewGame2(sceneId);
			},
			.onContinueGame = [sceneId]() -> void {
				hideLetterBox();
				ballModeSetPlayMode(sceneId);
			},
   	},
  });
  showLetterBoxHold("This is the intro mode", 0.f);
}


void startBallMode(objid sceneId){
  auto hasLevelSelect = gameapi -> getObjectsByAttr("levelselect", std::nullopt, sceneId).size() > 0;

  if (hasLevelSelect){
  	startBallIntroMode(sceneId);
  }else{
  	ballModeSetPlayMode(sceneId);
  }
}

void endBallMode(){
}

void doGravityHole(objid id, objid gravityHole){
  if (isControlledVehicle(id)){
  	auto& ballMode = getBallModeOptions();
		auto ballVehicle = getVehicleBall(vehicles, ballMode.ballId);
		setBallGravityWell(ballMode.ballId, *ballVehicle.value(), true, gravityHole);
  }
}


void deliverPowerup(objid vehicle, objid powerupId){
  auto& powerup = powerups.at(powerupId);
  if (powerup.lastRemoveTime.has_value()){
    return;
  }

  auto vehicleBall = getVehicleBall(vehicles, vehicle);
  modassert(vehicleBall.has_value(), "deliver powerup, not a ball");
  if (powerup.type == "jump"){
    setPowerupBall(*vehicleBall.value(), BIG_JUMP);
  }else if (powerup.type == "dash"){
    setPowerupBall(*vehicleBall.value(), LAUNCH_FORWARD);
  }else if (powerup.type == "low_gravity"){
    setPowerupBall(*vehicleBall.value(), LOW_GRAVITY);
  }else if (powerup.type == "teleport"){
    setPowerupBall(*vehicleBall.value(), TELEPORT);
  }else if (powerup.type == "invincibility"){
    setPowerupBall(*vehicleBall.value(), INVINCIBILITY);
  }else{
    modassert(false, std::string("invalid powerup type: ") + powerup.type);
    setPowerupBall(*vehicleBall.value(), std::nullopt);
  }

  playGameplayClipByIdCenter(getManagedSounds().teleportObjId.value(), std::nullopt, false);
  
  if(!powerup.respawnRateMs.has_value()){
    gameapi -> removeObjectById(powerupId);
  }else{
    powerup.lastRemoveTime = gameapi -> timeSeconds(false);
  }
}

void handleLevelEndCollision(int32_t obj1, int32_t obj2){
  bool didCollideLevelEnd = false;
  if (isControlledVehicle(obj1)){
    auto objAttr = getAttrHandle(obj2);
    auto playerEndAttr = getStrAttr(objAttr, "player_end");
    if (playerEndAttr.has_value()){
      didCollideLevelEnd = true;
    }
  }else if (isControlledVehicle(obj2)){
    auto objAttr = getAttrHandle(obj1);
    auto playerEndAttr = getStrAttr(objAttr, "player_end");
    if (playerEndAttr.has_value()){
      didCollideLevelEnd = true;
    }
  }

  std::cout << "handleLevelEndCollision: " << gameapi -> getGameObjNameForId(obj1).value() << ", " << gameapi -> getGameObjNameForId(obj2).value() << ", collide = " << didCollideLevelEnd << std::endl;
  if (didCollideLevelEnd){
    // obviously this is kind of overly coupled to the ball game here. 
    setBallLevelComplete();
  }
}

void handleGravityHoleCollision(objid obj1, objid obj2){
	return;
	
  {
    auto objAttr1 = getAttrHandle(obj1);
    auto gravityHole = getStrAttr(objAttr1, "gravityhole");
    if (gravityHole.has_value()){
      modlog("gravityHole 1: ", gameapi -> getGameObjNameForId(obj1).value() + ", " + gameapi -> getGameObjNameForId(obj2).value());
      doGravityHole(obj2, obj1);
    }
  }
   
  {

    auto objAttr2 = getAttrHandle(obj2);
    auto gravityHole = getStrAttr(objAttr2, "gravityhole");
    if (gravityHole.has_value()){
      modlog("gravityHole 2: ", gameapi -> getGameObjNameForId(obj1).value() + ", " + gameapi -> getGameObjNameForId(obj2).value());
      doGravityHole(obj1, obj2);
    }
  }
}

void handlePowerupCollision(int32_t obj1, int32_t obj2){
  if (isControlledVehicle(obj1)){
    auto objAttr = getAttrHandle(obj2);
    auto powerup = getStrAttr(objAttr, "powerup");
    if (powerup.has_value()){
      deliverPowerup(obj1, obj2);
    }
  }else if (isControlledVehicle(obj2)){
    auto objAttr = getAttrHandle(obj1);
    auto powerup = getStrAttr(objAttr, "powerup");
    if (powerup.has_value()){
      deliverPowerup(obj2, obj1); 
    }
  }
}

void doWorldSelect(objid id, std::string& value){
	std::cout << "worldView doWorldSelect called" << std::endl;
	auto& ballModeOptions = getBallModeOptions();
	ballModeOptions.worldView = WorldView {
		.world = value,
	};
}

void handleMultiselectCollision(int32_t obj1, int32_t obj2){
  auto nameObj1 = gameapi -> getGameObjNameForId(obj1).value();
  auto nameObj2 = gameapi -> getGameObjNameForId(obj2).value();

  //std::cout << "onModeCollision: " << nameObj1 << ", " << nameObj2 << std::endl;

	if (!inBallGametype()){
		return;
	}

	auto activeBall = getBallModeOptions().ballId;

  if (isControlledVehicle(obj1) && obj1 == activeBall){
    auto objAttr = getAttrHandle(obj2);
    auto worldSelect = getStrAttr(objAttr, "worldselect");
    if (worldSelect.has_value()){
		  std::cout << "onModeCollision: worldselect: " << nameObj2 << ", scene = " << gameapi -> listSceneId(obj2) << ", other: " << nameObj1 << "(" << obj1 <<  ")" << std::endl;
    	doWorldSelect(obj1, worldSelect.value());
    }
  }else if (isControlledVehicle(obj2) && obj2 == activeBall){
    auto objAttr = getAttrHandle(obj1);
    auto worldSelect = getStrAttr(objAttr, "worldselect");
    if (worldSelect.has_value()){
		  std::cout << "onModeCollision: worldselect: " << nameObj1 << ", scene = " << gameapi -> listSceneId(obj1) <<  ", other: " << nameObj2  << "(" << obj2 << ")" << std::endl;
    	doWorldSelect(obj2, worldSelect.value());
    }
  }
}

void deliverSoul(std::string& soul, objid soulId){
	auto& ballMode = getBallModeOptions();

	if (soul == "red"){
		ballMode.changeSpirit = MODE_RED;
	}else if (soul == "blue"){
		ballMode.changeSpirit = MODE_BLUE;
	}else if (soul == "yellow"){
		ballMode.changeSpirit = MODE_YELLOW;
	}else if (soul == "purple"){
		ballMode.changeSpirit = MODE_PURPLE;
	}
  gameapi -> removeObjectById(soulId);

}

void handleSoulCollision(int32_t obj1, int32_t obj2){
  if (isControlledVehicle(obj1)){
    auto objAttr = getAttrHandle(obj2);
    auto soul = getStrAttr(objAttr, "soul");
    if (soul.has_value()){
      deliverSoul(soul.value(), obj2);
    }
  }else if (isControlledVehicle(obj2)){
    auto objAttr = getAttrHandle(obj1);
    auto soul = getStrAttr(objAttr, "soul");
    if (soul.has_value()){
      deliverSoul(soul.value(), obj1);
    }
  }
}


void handleBallModeCollision(objid obj1, objid obj2){
  handleLevelEndCollision(obj1, obj2);
  handleGravityHoleCollision(obj1, obj2);
  handlePowerupCollision(obj1, obj2);

  handleMultiselectCollision(obj1, obj2);

  handleSoulCollision(obj1, obj2);

  // just check if it this the multiview here
}


std::vector<Narration> createNarration(std::vector<std::string> texts, int textLength, int* duration){
	std::vector<Narration> narrations;
	*duration = 0;

	int timeMs = 0;
	for (auto& text : texts){
			int startTime = timeMs;
			int endTime = startTime + textLength;
			timeMs = endTime;
 			Narration narration {
 				.startTimeMs = startTime,
 				.endTimeMs = endTime,
 				.text = text,
 			};
 			narrations.push_back(narration);
 			*duration = endTime;
	}
	return narrations;
}



struct DeathCutsceneData {
	std::optional<objid> narrationCutscene;
};
std::function<void(EasyCutscene&)> deathCutscene(){
 	int endTimeMs = 0;
 	auto narration = createNarration({ "I have fallen into this world", "No need to understand it", "I will try again" }, 5000, &endTimeMs);
 	float endTime = endTimeMs / 1000.f;

 	auto simpleNarrationCutscene = simpleNarration("The First Time I Died", narration);

	return [endTimeMs, endTime, simpleNarrationCutscene](EasyCutscene& cutscene) -> void {
		if(initialize(cutscene)){
		  playGameplayClipByIdCenter(getManagedSounds().teleportObjId.value(), std::nullopt, false);

 			auto& ballMode = getBallModeOptions();

		 	auto narrationCutscene = playCutscene(simpleNarrationCutscene, std::nullopt);
			std::cout << "death cutscene: initialize: t = " << gameapi -> timeSeconds(false) << std::endl;
			std::cout << "death cutscene: initialize: endTime = " << endTime << ", ms = " << endTimeMs << std::endl;

			DeathCutsceneData cutsceneData{};
			cutsceneData.narrationCutscene = narrationCutscene;
    	store(cutscene, cutsceneData);

		}
		if (finalize(cutscene)){
			std::cout << "death cutscene: finalize: t = " << gameapi -> timeSeconds(false) << std::endl;

			auto& ballOptions = getBallModeOptions();
			ballOptions.shouldReset = true;

 			auto sphereObj = ballOptions.rebirthSphere.value();
 			gameapi -> moveCameraTo(sphereObj, ballOptions.rebirthSphereInitialPos.value(), endTime);

 			setGameObjectLayer(ballOptions.eyeId, "");
 			setGameObjectLayer(ballOptions.rebirthSphere.value(), "");
		}


		DeathCutsceneData* cutsceneData = getStorage<DeathCutsceneData>(cutscene);
		modassert(cutsceneData, "no cutscene data");

  	if (glfwGetKey(window, 'K') && cutsceneData -> narrationCutscene.has_value()){
  		setCutsceneFinished(cutsceneData -> narrationCutscene.value());
  		cutsceneData -> narrationCutscene = std::nullopt;
  		setCutsceneFinished(cutscene);
  	}

		int initialDelay = 5000;


		waitUntil(cutscene, 1, initialDelay);
		run(cutscene, 2, []() -> void {
 			auto& ballMode = getBallModeOptions();
			auto rebirthSpherePos = gameapi -> getGameObjectPos(ballMode.rebirthSphere.value(), true, "[gamelogic] setToLevelEnd get sphere loc");
		 	//gameapi -> setGameObjectPosition(ballMode.ballId, rebirthSpherePos, true, Hint { .hint = "[gamelogic] - setToLevelEnd" });
			
	  	auto ballPos = gameapi -> getGameObjectPos(ballMode.ballId, true, "[gamelogic] get ball position for start");
		 	gameapi -> setGameObjectPosition(ballMode.rebirthSphere.value(), ballPos, true, Hint { .hint = "[gamelogic] - setToLevelEnd" });

 			setGameObjectLayer(ballMode.eyeId, "no_depth");
 			setGameObjectLayer(ballMode.rebirthSphere.value(), "no_depth");
		});
		waitUntil(cutscene, 3, initialDelay + 100);

		run(cutscene, 4, [endTime]() -> void {
			std::cout << "death cutscene: start move rebirth: t = " << gameapi -> timeSeconds(false) << std::endl;
 			auto& ballOptions = getBallModeOptions();
 			auto sphereObj = ballOptions.rebirthSphere.value();
 			gameapi -> moveCameraTo(sphereObj, ballOptions.playerSpawnPosition , endTime);
 			gameapi -> moveCameraTo(ballOptions.ballId, ballOptions.playerSpawnPosition , endTime);
		});

		waitUntil(cutscene, 5, initialDelay + endTimeMs + 100);


	};
}




void setToLevelEnd(){
	auto& ballMode = getBallModeOptions();

	// should this be here?
	//setGameObjectMeshEnabled(ballMode.ballId, true);
	setGameObjectTint(ballMode.eyeId, glm::vec4(1.f, 1.f, 1.f, 1.f));


 	auto cutscene = deathCutscene();
 	playCutscene(cutscene, std::nullopt);
}


GameTypeInfo getBallMode(){
	GameTypeInfo ballMode = GameTypeInfo {
	  .gametypeName = "ball",
	  .createGametype = [](void* data) -> std::any {
			BallModeOptions* modeOptionsPtr = static_cast<BallModeOptions*>(data);
			BallModeOptions& modeOptions = *modeOptionsPtr;
			{
				auto playerIndex = getDefaultPlayerIndex();
				auto entityId = getEntityForPlayerIndex(playerIndex).value();
				setEntityInVehicle(entityId, modeOptions.ballId);
				setCanExitVehicle(false);

				playCutscene(ballStartGameplay, std::nullopt);
			}

	  	auto pos = gameapi -> getGameObjectPos(modeOptions.ballId, true, "[gamelogic] get ball position for start");
	  	modeOptions.initialBallPos = pos;
	  	modeOptions.didLose = false;
	  	modeOptions.shouldReset = false;
	  	modeOptions.didReset = false;

	  	modassert(modeOptions.ballStartTime.has_value(), "no ball start time");
			getBallModeUI().value() -> ballMode.elapsedTime = []() -> float {
				auto& ballModeOptions  = getBallModeOptions();
				if (ballModeOptions.finalBallTime.has_value()){
					return ballModeOptions.finalBallTime.value();
				}
				return gameapi -> timeSeconds(false) - ballModeOptions.ballStartTime.value();
			};
			
	    return *modeOptionsPtr; 
	  },
	  .onEvent = [](std::any& gametype, std::string& event, std::any& value) -> void {},
	  .onKey = [](std::any& gametype, int key, int scancode, int action, int mods) -> void {
	  	BallModeOptions* ballModePtr = std::any_cast<BallModeOptions>(&gametype);
	  	modassert(ballModePtr, "ballMode options");
	  	BallModeOptions& ballMode = *ballModePtr;
 		  auto cameraId = ensureTempCamera(ballMode.sceneId);

	  	if (ballMode.worldView.has_value() && ballMode.worldView.value().onMultiview){
	  		if (key == 'U'){
	  			setTempCamera(std::nullopt, 0);
					hideLetterBox();
	  			removeCameraFromMultiOrbView(cameraId);
		      setEntityControlDisabled(false, getEntityForPlayerIndex(0).value());
					
					std::cout << "worldView set null 1" << std::endl;
	  			ballMode.worldView = std::nullopt;

		  		auto ballVehicle = getVehicleBall(vehicles, ballMode.ballId).value();
	  			setDisableAutolaunch(*ballVehicle, false);
	  		}
	  		if (key == 'A'){
	  			auto multiOrbView = multiorbViewByCamera(cameraId);
	  			if (multiOrbView.has_value()){
		  			prevOrb(*multiOrbView.value());
	  			}
	  		}
	  		if (key == 'D'){
	  			auto multiOrbView = multiorbViewByCamera(cameraId);
	  			if (multiOrbView.has_value()){
		  			nextOrb(*multiOrbView.value());  			
	  			}
	  		}
	  		if (key == 32){
	   			setTempCamera(std::nullopt, 0);
					hideLetterBox();
					ballMode.worldView = std::nullopt;
					goToLevel("dev");
					return;

	  			auto multiOrbView = multiorbViewByCamera(cameraId);
	  			if (multiOrbView.has_value()){
	  				auto level = getSelectedLevel(*multiOrbView.value());
	  				if (level.has_value()){
							std::cout << "worldView set null 2" << std::endl;
	  					goToLevel(level.value());
	  				}
	  			}
	  		}
	  	}

	  	if (key == 'Z' && action == 1){
	  		ballMode.changeSpirit = MODE_NONE;
	  	}
	  	if (key == 'X' && action == 1){
	  		ballMode.changeSpirit = MODE_RED;
	  	}
	  	if (key == 'C' && action == 1){
	  		ballMode.changeSpirit = MODE_BLUE;
	  	}
	  	if (key == 'V' && action == 1){
	  		ballMode.changeSpirit = MODE_YELLOW;
	  	}
	  	if (key == 'B' && action == 1){
	  		ballMode.changeSpirit = MODE_PURPLE;
	  	}
	  	if (key == 'N' && action == 1){
	  		ballMode.shouldReset = true;
	  	}

	  	std::cout << "ball mode: " << key << ", " << action << std::endl;
	  },
	  .onFrame = [](std::any& gametype) -> void {
	  	BallModeOptions* ballModePtr = std::any_cast<BallModeOptions>(&gametype);
	  	modassert(ballModePtr, "ballMode options");
	  	BallModeOptions& ballMode = *ballModePtr;

	  	if (ballMode.worldView.has_value() && !ballMode.worldView.value().didChangeToOrb && !ballMode.worldView.value().onMultiview){
	  		auto cameraId = ensureTempCamera(ballMode.sceneId);

	  		auto viewTransform = gameapi -> getCameraTransform(getDefaultPlayerIndex());
				auto viewPosition = viewTransform.position;
				auto viewRot = viewTransform.rotation;
		  	gameapi -> setGameObjectPosition(cameraId, viewPosition, true, Hint { .hint = "[gamelogic] - ball set pos" });
		  	gameapi -> setGameObjectRot(cameraId, viewRot, true, Hint { .hint = "[gamelogic] - ball set pos" });

	  		ballMode.worldView.value().didChangeToOrb = true;
	  		return;
	  	}
	  	if (ballMode.worldView.has_value() &&  !ballMode.worldView.value().onMultiview) {
	  		ballMode.worldView.value().didChangeToOrb = false;
	  		ballMode.worldView.value().onMultiview = true;
	  		auto cameraId = ensureTempCamera(ballMode.sceneId);
   			setTempCamera(cameraId, 0);

   			static int multiViewCount = 0;
 			  showLetterBoxHold(std::string("This is the multiview: ") + std::to_string(multiViewCount++), 0.f);
	      setEntityControlDisabled(true, getEntityForPlayerIndex(0).value());

				setToMultiOrbView(cameraId, ballMode.worldView.value().world);
				//setToMultiOrbView(cameraId, ballMode.worldView.value().world);

				auto gravityWellId = gravityWellForName(ballMode.worldView.value().world);
				if (!gravityWellId.has_value()){
					return;
				}
				//modassert(gravityWellId.has_value(), std::string("no gravity well for name: ") + ballMode.worldView.value().world);
				doGravityHole(ballMode.ballId, gravityWellId.value());

	  		auto ballVehicle = getVehicleBall(vehicles, ballMode.ballId).value();
				setDisableAutolaunch(*ballVehicle, true);

				return;
	  	}

	  	// below is the ball game logic itself
	  		std::cout << "ball onframe" << std::endl;

	  		if (ballMode.changeSpirit.has_value()){
	  			ballMode.spirit = ballMode.changeSpirit.value();
	  			ballMode.changeSpirit = std::nullopt;

	  			int mask = 0b11111111;
					if (ballMode.spirit == MODE_RED){
						mask = 0b0001;
					}else if (ballMode.spirit == MODE_YELLOW){
						mask = 0b0010;
					}else if (ballMode.spirit == MODE_BLUE){
						mask = 0b0100;
					}else if (ballMode.spirit == MODE_PURPLE){
						mask = 0b1000;
					}
				  setEntityActivateMask(getEntityForPlayerIndex(0).value(), mask);

				  if (ballMode.spirit == MODE_BLUE){
				  	setActivatableObject(ballMode.ballId);
				  }else{
					  setActivatableObject(std::nullopt);
				  }

  			  playGameplayClipByIdCenter(getManagedSounds().teleportObjId.value(), std::nullopt, false);

  			  
	  			if (ballMode.spirit == MODE_NONE){
	  				setGameObjectEmitterEffectTint(ballMode.spiritId, glm::vec4(0.f, 0.f, 0.f, 1.f));
	  			}else if (ballMode.spirit == MODE_RED){
	  				setGameObjectEmitterEffectTint(ballMode.spiritId, glm::vec4(1.f, 0.f, 0.f, 1.f));
	  			}else if (ballMode.spirit == MODE_BLUE){
	  				setGameObjectEmitterEffectTint(ballMode.spiritId, glm::vec4(0.f, 0.f, 1.f, 1.f));
	  			}else if (ballMode.spirit == MODE_YELLOW){
	  				setGameObjectEmitterEffectTint(ballMode.spiritId, glm::vec4(1.f, 1.f, 0.f, 1.f));
	  			}else if (ballMode.spirit == MODE_PURPLE){
	  				setGameObjectEmitterEffectTint(ballMode.spiritId, glm::vec4(1.f, 0.f, 1.f, 1.f));
	  			}
	  		}


	  		if (isInKillPlane(ballMode.ballId) && !ballMode.didLose && !ballMode.didReset){ // didReset b/c otherwise at same pos
		  		auto ballVehicle = getVehicleBall(vehicles, ballMode.ballId);
	  			if (ballMode.spirit != MODE_YELLOW && !(ballVehicle.value() -> powerup.has_value() && ballVehicle.value() -> powerup.value().powerup == INVINCIBILITY && ballVehicle.value() -> powerup.value().useTime.has_value())){
	  				ballMode.didLose = true;
	  				auto position = gameapi -> getGameObjectPos(ballMode.ballId, true, "[gamelogic] - ballIntroOpening pos");
	  				
	  				//position = glm::vec3(100.f, 100.f, 100.f);

	  				createExplosion(position, 5.f, 0.f);
	  				setGameObjectMeshEnabled(ballMode.ballId, false);
	  				setGameObjectPhysicsEnable(ballMode.ballId, false);
		  			handleRemoveKillplaneCollision(ballMode.ballId);

		  			setGameObjectTint(ballMode.eyeId, glm::vec4(0.f, 0.f, 0.f, 1.f));
	  			
		  			setToLevelEnd();
	  			}
	  		}

	  		if (ballMode.didReset){ 
	  			ballMode.didReset = false;
		  		setGameObjectMeshEnabled(ballMode.ballId, true);
		  		setGameObjectPhysicsEnable(ballMode.ballId, true);
	  			setGameObjectTint(ballMode.eyeId, glm::vec4(1.f, 1.f, 1.f, 1.f));
	  		}
	  		if (ballMode.shouldReset){
		  		gameapi -> setGameObjectPosition(ballMode.ballId, ballMode.initialBallPos.value(), true, Hint { .hint = "[gamelogic] - ball set pos" });
		  		ballMode.didLose = false;
					ballMode.shouldReset = false;
					ballMode.ballStartTime = gameapi -> timeSeconds(false);
					ballMode.didReset = true;
	  		}

	  		auto powerup = getPowerup();
	  		if (powerup.has_value()){
		  		std::cout << "powerup: " << print(powerup.value().powerup) << std::endl;
	  			if (powerup.value().powerup == BIG_JUMP){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/jump.png", std::nullopt, std::nullopt);
	 		  		return;
	  			}else if (powerup.value().powerup == LAUNCH_FORWARD){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/dash.png", std::nullopt, std::nullopt);
	 		  		return;
	  			}else if (powerup.value().powerup == LOW_GRAVITY){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/bounce.png", std::nullopt, std::nullopt);
	 		  		return;
	  			}else if (powerup.value().powerup == REVERSE_GRAVITY){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/bounce.normal.png", std::nullopt, std::nullopt);
	 		  		return;
	  			}else if (powerup.value().powerup == TELEPORT){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/teleport.png", std::nullopt, std::nullopt);
	 		  		return;
	  			}else if (powerup.value().powerup == INVINCIBILITY){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/teleport.normal.png", powerup.value().useTime, powerup.value().duration);
	 		  		return;
	  			}else{
	  				modassert(false, "invalid powerup size ball.cpp");
	  			}
	  		}
	  	

  		setPowerupTexture("../gameresources/build/textures/ballgame/none.png", std::nullopt, std::nullopt);

  		if (ballMode.didLose){
	  		//gameapi -> drawText("you lose", 0.f, 0.f, 12, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  		}

	  },
	};
	return ballMode;
}


/////////////////////////////////////////////////////////

