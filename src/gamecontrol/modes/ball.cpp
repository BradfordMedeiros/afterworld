#include "./ball.h"

extern CustomApiBindings* gameapi;

extern GameTypes gametypeSystem;
extern std::optional<std::string> activeLevel;
extern Vehicles vehicles;
extern Movement movement;
extern Weapons weapons;
extern std::unordered_map<objid, Powerup> powerups;

void goToLevel(std::string levelShortName);
void setPauseMenuOverride(std::optional<std::function<void()>> goToMenuFn);
void inputOverride(bool paused, bool showMouse);
void inputOverride();
bool isReloadKey(int button);
void handleRemoveKillplaneCollision(objid);

struct BallModeOptions{
   std::optional<glm::vec3> initialBallPos;
   objid ballId;
   objid sceneId;
   bool didLose = false;
   bool shouldReset = false;
   bool didReset = false;
   std::optional<float> ballStartTime;
   std::optional<float> finalBallTime;


   bool shouldChangeToOrb = false;
   bool didChangeToOrb = false;
   bool levelSelect = false;
};

BallModeOptions& getBallModeOptions(){
	auto data = getGametypeData(gametypeSystem);
	modassert(data.has_value(), "getBallMode options - gametype system empty data");
 	BallModeOptions* ballModePtr = std::any_cast<BallModeOptions>(data.value());
 	modassert(ballModePtr, "no active ball mode");
 	return *ballModePtr;
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

std::optional<std::string> getRailName(){
	std::string railName("levelintro");
	auto rail = railIdForName(railName);
	if (!rail.has_value()){
		return std::nullopt;
	}
	return railName;
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

  		auto rail = getRailName();
  		if (!rail.has_value()){
		  	BallStartData* startData = getStorage<BallStartData>(cutscene);
		  	modassert(startData, "ballStartGameplay startData is null");
		  	startData -> cutsceneFinished = true;
   			setTempCamera(std::nullopt, 0);
   			return;
  		}

  		setTempCamera(ensureTempCamera(sceneId), 0);
  		
  		std::vector<SimpleNarration> narrations;

  		{
  			auto railId = railIdForName(rail.value());
  			auto& railPoints = *railForId(railId.value()).value();
  			for (auto& key : railPoints.keys){
  				narrations.push_back(SimpleNarration {
  					.text = key.has_value() ? key.value() : "",
  				});
  			}
  		}

  		NarratedMovement narratedMovement {
  			.rail = rail.value(),
  			.letterbox = "Welcome to The World",
  			.narrations = narrations,
  		};
 			playCutscene(simpleNarratedMovement2(ensureTempCamera(sceneId), narratedMovement, false, [&cutscene]() -> void { 
		  	BallStartData* startData = getStorage<BallStartData>(cutscene);
		  	modassert(startData, "ballStartGameplay startData is null");
		  	startData -> cutsceneFinished = true;
   			setTempCamera(std::nullopt, 0);
			}), std::nullopt);
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

objid createBallObj(objid sceneId, glm::vec3 position){
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
 	auto ball = gameapi -> makeObjectAttr(sceneId, std::string("ball1"), attr, submodelAttributes);  

  bool addParticle = true;

  if (addParticle){
  
  	{
  		modassert(ball.has_value(), "ball was not created");
  		GameobjAttributes emitterAttr { 
  			.attr = {
 					{ "effekseer", "./res/particles/spirit.efkefc" },
 					{ "state", "enabled" },
  			} 
  		};
  		std::unordered_map<std::string, GameobjAttributes> submodelAttributesEmitter;
  		auto ballSpirit = gameapi -> makeObjectAttr(sceneId, std::string("+ballSpirit"), emitterAttr, submodelAttributesEmitter);
  		modassert(ballSpirit.has_value(), "ballSpirit was not created");
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

	return ball.value();
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

  //////////


	auto ballId = createBallObj(sceneId, playerSpawnPosition);


	GameTypeInfo ballMode = getBallMode();
	BallModeOptions modeOptions {};
	modeOptions.ballId = ballId;
	modeOptions.ballStartTime = gameapi -> timeSeconds(false);
	modeOptions.sceneId = sceneId;

	changeGameType(gametypeSystem, ballMode, "ball", &modeOptions);
}

void ballModeNewGame2(objid sceneId){
  resetProgress();
  playGameplayClipById(getManagedSounds().teleportObjId.value(), std::nullopt, std::nullopt, false);

  inputOverride(false, false);
	changeUiMode(UiModeNone{});

  NarratedMovement narratedMovement {
  	.rail = "intro",
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
//
  auto cameraId = ensureTempCamera(sceneId);
	setTempCamera(cameraId, 0);

  auto currentCutscene = playCutscene(simpleNarratedMovement2(cameraId, narratedMovement, false, [sceneId]() -> void { 
  		ballModeSetPlayMode(sceneId); 
  }), std::nullopt);
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
  showLetterBoxHold("", 0.f);
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

void handleBallModeCollision(objid obj1, objid obj2){
  handleLevelEndCollision(obj1, obj2);
  handleGravityHoleCollision(obj1, obj2);
  handlePowerupCollision(obj1, obj2);
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
	  	if(isReloadKey(key) && action == 0){
	  		//ballMode.shouldReset = true;

	  			ballMode.shouldChangeToOrb = true;

	  	}

 		  auto cameraId = ensureTempCamera(ballMode.sceneId);
	  	if (isToggleThirdPersonKey(key) && action == 0){
	  		//ballMode.didLose = true;
	  		//auto position = gameapi -> getGameObjectPos(ballMode.ballId, true, "[gamelogic] - ballIntroOpening pos");
	  		//createExplosion(position, 5.f, 0.f);
	  		setTempCamera(std::nullopt, 0);
	  		removeCameraFromMultiOrbView(cameraId);
	  	}

	  	if (key == 'I' && action == 1){
	  		//ballMode.levelSelect = false;
	  		auto multiOrbViewPtr = multiorbViewByCamera(cameraId).value();
	  		auto inOverWorld = isOverworld(*multiOrbViewPtr);
	  		goToOverWorld(*multiOrbViewPtr, !inOverWorld);
	  	}
	  	if (key == 'T' && action == 1){
	  		auto multiOrbViewPtr = multiorbViewByCamera(cameraId).value();
	  		nextOrb(*multiOrbViewPtr);
	  	}
	  	if (key == 'Y' && action == 1){
	  		auto multiOrbViewPtr = multiorbViewByCamera(cameraId).value();
	  		prevOrb(*multiOrbViewPtr);
	  	}
	  	if (key == 'U'){
	  		auto multiOrbViewPtr = multiorbViewByCamera(cameraId).value();
	  		auto level = getSelectedLevel(*multiOrbViewPtr);
	  		if (level.has_value()){
	  			goToLevel(level.value());
	  		}
	  	}
	  	
	  	std::cout << "ball mode: " << key << ", " << action << std::endl;
	  },
	  .onFrame = [](std::any& gametype) -> void {
	  	BallModeOptions* ballModePtr = std::any_cast<BallModeOptions>(&gametype);
	  	modassert(ballModePtr, "ballMode options");
	  	BallModeOptions& ballMode = *ballModePtr;

	  	if (ballMode.shouldChangeToOrb){
	  		auto cameraId = ensureTempCamera(ballMode.sceneId);

	  		auto viewTransform = gameapi -> getCameraTransform(getDefaultPlayerIndex());
				auto viewPosition = viewTransform.position;
				auto viewRot = viewTransform.rotation;
		  	gameapi -> setGameObjectPosition(cameraId, viewPosition, true, Hint { .hint = "[gamelogic] - ball set pos" });
		  	gameapi -> setGameObjectRot(cameraId, viewRot, true, Hint { .hint = "[gamelogic] - ball set pos" });

	  		ballMode.shouldChangeToOrb = false;
	  		ballMode.didChangeToOrb = true;
	  		return;
	  	}
	  	if (ballMode.didChangeToOrb){
	  		ballMode.didChangeToOrb = false;
	  		auto cameraId = ensureTempCamera(ballMode.sceneId);
   			setTempCamera(cameraId, 0);
				setToMultiOrbView(cameraId, std::nullopt);

				ballMode.levelSelect = true;
				return;
	  	}

	  	// below is the ball game logic itself
	  		std::cout << "ball onframe" << std::endl;
	  		if (isInKillPlane(ballMode.ballId) && !ballMode.didLose && !ballMode.didReset){ // didReset b/c otherwise at same pos
		  		auto ballVehicle = getVehicleBall(vehicles, ballMode.ballId);
	  			if (!(ballVehicle.value() -> powerup.has_value() && ballVehicle.value() -> powerup.value().powerup == INVINCIBILITY && ballVehicle.value() -> powerup.value().useTime.has_value())){
	  				ballMode.didLose = true;
	  				auto position = gameapi -> getGameObjectPos(ballMode.ballId, true, "[gamelogic] - ballIntroOpening pos");
	  				createExplosion(position, 5.f, 0.f);
	  				setGameObjectMeshEnabled(ballMode.ballId, false);
	  				setGameObjectPhysicsEnable(ballMode.ballId, false);
		  			handleRemoveKillplaneCollision(ballMode.ballId);
	  			}
	  		}

	  		if (ballMode.didReset){ 
	  			ballMode.didReset = false;
		  		setGameObjectMeshEnabled(ballMode.ballId, true);
		  		setGameObjectPhysicsEnable(ballMode.ballId, true);
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
	  		gameapi -> drawText("you lose", 0.f, 0.f, 12, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  		}
	  },
	};
	return ballMode;
}


/////////////////////////////////////////////////////////

