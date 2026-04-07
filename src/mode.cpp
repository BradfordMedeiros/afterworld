#include "./mode.h"

extern CustomApiBindings* gameapi;

extern GameTypes gametypeSystem;
extern std::optional<std::string> activeLevel;
extern Vehicles vehicles;
extern GLFWwindow* window;

std::optional<objid> currentCutscene;

void setCanExitVehicle(bool canExit);
void enterVehicleRaw(int playerIndex, objid vehicleId, objid id);
void goToLevel(std::string levelShortName);
bool isReloadKey(int button);
void setLifetimeObject(objid id, std::function<void()> fn, std::string hint);
void createExplosion(glm::vec3 position, float outerRadius, float damage);
void handleRemoveKillplaneCollision(objid);

void startRotate(objid id);
void stopRotate(objid id);

bool inBallMode = false;
std::optional<float> ballStartTime;
std::optional<float> finalBallTime;

void ballStartGameplay(EasyCutscene& cutscene){
  if (initialize(cutscene)){
		setDisablePlayerControl(true, 0);
  }
  if (finalize(cutscene)){
  }

  waitUntil(cutscene, 0, 500);
  run(cutscene, 1, []() -> void {
    showLetterBox("Learning to Roll", 10.f);
  });
  run(cutscene, 2, []() -> void {
  	setDisablePlayerControl(false, 0);
  });
}
void ballEndGameplay(EasyCutscene& cutscene){
	if (initialize(cutscene)){
	  playGameplayClipById(getManagedSounds().teleportObjId.value(), std::nullopt, std::nullopt, false);
		setDisablePlayerControl(true, 0);
	}
  waitUntil(cutscene, 0, 2000);

  if (finishedThisFrame(cutscene, 0)){
		setShowBallOptions(
				BallComponentOptions {
					.levelComplete = BallLevelComplete{},
				}
		);
  }

	waitFor(cutscene, 1, []() -> bool {
		return getGlobalState().leftMouseDown;
	});

  run(cutscene, 2, []() -> void {
 		setShowBallOptions(std::nullopt);
  	goToLevel("ballselect");
  	ballModeLevelSelect();
  });
}

void setBallLevelComplete(){
	if (!inBallMode){
		return;
	}
	inBallMode = false;
	std::cout << "set ball level complete: " << activeLevel.value() << std::endl;

	auto timeElapsed = gameapi -> timeSeconds(false) - ballStartTime.value();

	changeGameType(gametypeSystem, NULL, NULL);
	markLevelComplete(activeLevel.value(), timeElapsed);
	finalBallTime = timeElapsed;

	commitCrystals();

	playCutscene(ballEndGameplay, std::nullopt);	
}

void createBallObj(objid sceneId, glm::vec3 position){
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


  /*
  			{ "clip", "../gameresources/sound/rain.wav" },
			{ "loop", "true" },
			*/
}

std::optional<objid> getBallId(){
	  auto vehicles = getVehicleIds();
 	  modassert(vehicles.size() == 1, "invalid expected vehicle size");
	  return vehicles.at(0);
}

void changeUi(bool showBallUi){
		if (showBallUi){
			setShowBallOptions(
				BallComponentOptions {
					.powerupTexture = "../gameresources/build/textures/ballgame/jump.png",
				}
			);
		}else{
			setShowBallOptions(std::nullopt);
		}
}

void showTimeElapsed(bool shouldShow){
		if (!showBallOptions().has_value() || !ballStartTime.has_value()){
			return;
		}
		auto ballOptions = showBallOptions().value();
		if (!shouldShow){
			ballOptions.elapsedTime = std::nullopt;
		}else{
			ballOptions.elapsedTime = []() -> float {
				if (finalBallTime.has_value()){
					return finalBallTime.value();
				}
				return gameapi -> timeSeconds(false) - ballStartTime.value();
			};
		}
		
		setShowBallOptions(ballOptions);
}

void setPowerupTexture(std::string texture, std::optional<float> startTime, std::optional<float> duration){
	auto ballOptions = showBallOptions();
	if (!ballOptions.has_value()){
		return;
	}
	if (ballOptions.has_value()){
		auto newBallOptions = ballOptions.value();
		newBallOptions.powerupTexture = texture;
		newBallOptions.powerupStartTime = startTime;
		newBallOptions.powerupDuration = duration;
		setShowBallOptions(newBallOptions);
	}
}

std::optional<BallPowerupState> getPowerup(){
	auto activePlayer = getActivePlayerId(0);
	auto vehicleIds = getVehicleIds();
	if (vehicleIds.size() == 0){
		return std::nullopt;
	}

	auto vehicleBall = getVehicleBall(vehicles, vehicleIds.at(0));
	modassert(vehicleBall.has_value(), "get powerup - not a ball");
	return getBallPowerup(*vehicleBall.value());
} 

void startBallMode(objid sceneId){
	inBallMode = true;
	ballStartTime = gameapi -> timeSeconds(false);

	auto playerSpawnId = findObjByShortName("playerspawn", std::nullopt);
	auto position = gameapi -> getGameObjectPos(playerSpawnId.value(), true, "[gamelogic] ball - get playerspawn position");
	createBallObj(sceneId, position);
	BallModeOptions modeOptions {};
	changeGameType(gametypeSystem, "ball", &modeOptions);
	setHudEnabled(false);
}

void endBallMode(){
	inBallMode = false;
	ballStartTime = std::nullopt;
	finalBallTime = std::nullopt;

	setCanExitVehicle(true);
	setShowBallOptions(std::nullopt);
	changeGameType(gametypeSystem, NULL, NULL);
	setHudEnabled(true);
}

struct GravityHole {
	objid gravityHoleId;
};
void gravityHoleBall(objid gravityHoleId){
	gameapi -> sendNotifyMessage("ballgravity", GravityHole{
		.gravityHoleId = gravityHoleId,
	});
}

GameTypeInfo getBallMode(){
	GameTypeInfo ballMode = GameTypeInfo {
	  .gametypeName = "ball",
	  .events = { "ball", "ballgravity" },
	  .createGametype = [](void* data) -> std::any {
			BallModeOptions* modeOptions = static_cast<BallModeOptions*>(data);
			{
				auto activePlayer = getActivePlayerId(0);
				auto vehicles = getVehicleIds();
				modassert(vehicles.size() == 1, "invalid expected vehicle size");
				modassert(activePlayer.has_value(), "ball set active player no active player");
				std::cout << "vehicles: " << print(vehicles) << "[" << gameapi -> getGameObjNameForId(vehicles.at(0)).value()  << "]" << ", playerid: " << print(activePlayer) << ", [" << gameapi -> getGameObjNameForId(activePlayer.value()).value()  <<  "]" << std::endl;
				std::cout << "vehicles: " << gameapi -> getGameObjNameForId(gameapi -> getActiveCamera(std::nullopt).value()).value()  << std::endl;
				enterVehicleRaw(0, vehicles.at(0), activePlayer.value());
				setCanExitVehicle(false);

				playCutscene(ballStartGameplay, std::nullopt);
			}

		  auto ballId = getBallId().value();
	  	auto pos = gameapi -> getGameObjectPos(ballId, true, "[gamelogic] get ball position for start");
	  	modeOptions -> initialBallPos = pos;
	  	modeOptions -> ballId = ballId;
	  	modeOptions -> didLose = false;
	  	modeOptions -> shouldReset = false;
	  	modeOptions -> didReset = false;

			changeUi(true);
 	   	showTimeElapsed(true);

	    return *modeOptions; 
	  },
	  .onEvent = [](std::any& gametype, std::string& event, std::any& value) -> bool {
	  	BallModeOptions* ballMode = std::any_cast<BallModeOptions>(&gametype);
	  	modassert(ballMode, "ballMode options");

	  	if (event == "ballgravity"){
		  	GravityHole* gravityHole = std::any_cast<GravityHole>(&value);
		  	modassert(gravityHole, "expected type for ballgravity");
	  		auto ballVehicle = getVehicleBall(vehicles, ballMode -> ballId.value());
	  		setBallGravityWell(ballMode -> ballId.value(), *ballVehicle.value(), true, gravityHole -> gravityHoleId);
	  		return false;
	  	}

	  	std::string* message = std::any_cast<std::string>(&value);
	  	modassert(message, "invalid type ball-mode");
	  	std::cout << "from ball mode: " << event << ", " << *message << std::endl;

	  	if (*message == "reset"){
	  		modassert(ballMode -> initialBallPos.has_value(), "no initial ball position");
	  		ballMode -> shouldReset = true;
	  	}

	   	return false;
	  },
	  .getDebugText = [](std::any& gametype) -> std::string {
	    return std::string("ball mode");
	  },
	  .getScoreInfo = [](std::any& gametype, float startTime) -> std::optional<GametypeData> { return std::nullopt; },
	  .onKey = [](std::any& gametype, int key, int scancode, int action, int mods) -> void {
	  	BallModeOptions* ballMode = std::any_cast<BallModeOptions>(&gametype);
	  	modassert(ballMode, "ballMode options");
	  	if(isReloadKey(key) && action == 0){
	  		gameapi -> sendNotifyMessage("ball", std::string("reset"));
	  	}
	  	if (isToggleThirdPersonKey(key) && action == 0){
	  		ballMode -> didLose = true;
	  		auto position = gameapi -> getGameObjectPos(ballMode -> ballId.value(), true, "[gamelogic] - ballIntroOpening pos");
	  		createExplosion(position, 5.f, 0.f);
	  	}
	  	std::cout << "ball mode: " << key << ", " << action << std::endl;
	  },
	  .onFrame = [](std::any& gametype) -> void {
	  	BallModeOptions* ballMode = std::any_cast<BallModeOptions>(&gametype);
	  	modassert(ballMode, "ballMode options");
	  	std::cout << "ball onframe" << std::endl;
	  	if (ballMode -> ballId.has_value()){
	  		if (isInKillPlane(ballMode -> ballId.value()) && !ballMode -> didLose && !ballMode -> didReset){ // didReset b/c otherwise at same pos
		  		auto ballVehicle = getVehicleBall(vehicles, ballMode -> ballId.value());
	  			if (!(ballVehicle.value() -> powerup.has_value() && ballVehicle.value() -> powerup.value().powerup == INVINCIBILITY && ballVehicle.value() -> powerup.value().useTime.has_value())){
	  				ballMode -> didLose = true;
	  				auto position = gameapi -> getGameObjectPos(ballMode -> ballId.value(), true, "[gamelogic] - ballIntroOpening pos");
	  				createExplosion(position, 5.f, 0.f);
	  				setGameObjectMeshEnabled(ballMode -> ballId.value(), false);
	  				setGameObjectPhysicsEnable(ballMode -> ballId.value(), false);
		  			handleRemoveKillplaneCollision(ballMode -> ballId.value());
	  			}

	  		}

	  		if (ballMode -> didReset){ 
	  			ballMode -> didReset = false;
		  		setGameObjectMeshEnabled(ballMode -> ballId.value(), true);
		  		setGameObjectPhysicsEnable(ballMode -> ballId.value(), true);
	  		}
	  		if (ballMode -> shouldReset){
		  		gameapi -> setGameObjectPosition(ballMode -> ballId.value(), ballMode -> initialBallPos.value(), true, Hint { .hint = "[gamelogic] - ball set pos" });
		  		ballMode -> didLose = false;
					ballMode -> shouldReset = false;
					ballStartTime = gameapi -> timeSeconds(false);
					ballMode -> didReset = true;
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
	  	}

  		setPowerupTexture("../gameresources/build/textures/ballgame/none.png", std::nullopt, std::nullopt);

  		if (ballMode -> didLose){
	  		gameapi -> drawText("you lose", 0.f, 0.f, 12, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  		}
	  },
	};
	return ballMode;
}

/////////////////////////////////////////////////////////
struct IntroModeOptions {
   objid cameraId;
   int activeLayer;
};

static int activeLayer = 0;
static std::string activeWorldName;
std::optional<objid> orbCameraId;

void startIntroMode(objid sceneId){
	if (currentCutscene.has_value()){
		removeCutscene(currentCutscene.value(), true);
		currentCutscene = std::nullopt;
	}
  auto cameraId = findObjByShortName(">menu-view", sceneId);
	removeCameraFromOrbView(cameraId.value());
  setTempCamera(cameraId.value(), 0);
  setHudEnabled(false);
  setShowLiveMenu(true);
  showLetterBoxHold("", 0.f);
  orbCameraId = cameraId;

  IntroModeOptions modeOptions {
  	.cameraId = cameraId.value(),
  };
	changeGameType(gametypeSystem, "ball-intro", &modeOptions);
}
void endIntroMode(){
	changeGameType(gametypeSystem, NULL, NULL);
  setShowLiveMenu(false);
  if (currentCutscene.has_value()){
    removeCutscene(currentCutscene.value());
  }
}

struct BallIntroData {
	bool showText;
	glm::vec3 initialPos;
	objid cameraId;
	std::vector<int> times;
	int railLengthMs;
};

struct CutsceneOption {
	std::vector<std::string> text;
	std::string rail;
	std::string letterbox;

	// state, not config data 
	bool hasAlreadyPlayed = false;
};

struct LevelOrbNavInfo {
	std::string orbUi;
	std::optional<int> orbIndex;
	std::optional<int> maxCompletedIndex;
};

struct LevelOrbLayer {
	std::string orbUi;
};

std::vector<LevelOrbLayer> orbLayers {
	LevelOrbLayer {
		.orbUi = "testorb",
	},
	LevelOrbLayer {
		.orbUi = "testorb3",
	},
	LevelOrbLayer {
		.orbUi = "metaworld",
	},
};

std::string currentOverworldName(){
	return activeWorldName;
}

bool isOverworld(){
	return activeLayer == (orbLayers.size() - 1);
}

void goToOverWorld(){
	activeWorldName = orbLayers.at(activeLayer).orbUi;
	activeLayer = orbLayers.size() - 1;
}

LevelOrbNavInfo getLevelOrbInfo(objid cameraId){
	auto& orbLayer = orbLayers.at(activeLayer);
  auto orbUi = orbUiByName(orbLayer.orbUi);
 	auto orbIndex = getActiveOrbViewIndex(cameraId);

 	auto metaworldIndex = getMaxCompleteOrbIndex(*orbUi.value());
 	if (orbLayer.orbUi == "metaworld"){
 		auto& orbs = orbUi.value() -> orbs;
 		for (auto& orb : orbs){
 			if (orb.level == activeWorldName){
 				metaworldIndex = orb.index;
 			}
 		}
 	}

	return LevelOrbNavInfo {
		.orbUi = orbLayer.orbUi,
		.orbIndex = orbIndex,
		.maxCompletedIndex = orbLayer.orbUi == "metaworld" ? metaworldIndex : getMaxCompleteOrbIndex(*orbUi.value()),
	};
}

std::optional<std::string> overworldLevel(){
	if (!orbCameraId.has_value()){
		return std::nullopt;
	}
	std::optional<Orb*> orb = selectedOrbForCamera(orbCameraId.value());
	if (!orb.has_value()){
		return std::nullopt;
	}
	return orb.value() -> level;
}

void onModeOrbSelect(std::vector<OrbSelection>& selectedOrbs){
	if (!orbCameraId.has_value()){
		return;
	}

	for (auto& selectedOrb : selectedOrbs){
		if (selectedOrb.cameraId == orbCameraId.value()){
			if (isOverworld()){
				if (selectedOrb.moveRightKey){
					OrbView& orbView = *selectedOrb.orbView;
					setOrbSelectIndex(orbView, getNextOrbIndex(*selectedOrb.orbUi, orbView.targetIndex));
					auto orb = selectedOrbForCamera(orbCameraId.value());
					if (orb.has_value()){
						activeWorldName = orb.value() -> level;
					}
				}else if (selectedOrb.moveLeftKey){
					OrbView& orbView = *selectedOrb.orbView;
					setOrbSelectIndex(orbView, getPrevOrbIndex(*selectedOrb.orbUi, orbView.targetIndex));
					auto orb = selectedOrbForCamera(orbCameraId.value());
					if (orb.has_value()){
						activeWorldName = orb.value() -> level;
					}
				}

				if (selectedOrb.selectKey && selectedOrb.currentOrb.has_value() && activeLayer == orbLayers.size() - 1){
					for (int i = 0; i < orbLayers.size() - 1; i++){
						if (orbLayers.at(i).orbUi == selectedOrb.currentOrb.value() -> level){
							activeWorldName = selectedOrb.currentOrb.value() -> level;
							activeLayer = i;
						}
					}
					return;
				}
			}else{
				if (selectedOrb.moveRightNoSpace){
  		  	goToOverWorld();
  			}
  			if (selectedOrb.moveLeftNoSpace){
  			  goToOverWorld();
  			}
  			if (selectedOrb.optionKey){
  				goToOverWorld();
  			}

				if (selectedOrb.moveRightKey){
					OrbView& orbView = *selectedOrb.orbView;
					setOrbSelectIndex(orbView, getNextOrbIndex(*selectedOrb.orbUi, orbView.targetIndex));
				}else if (selectedOrb.moveLeftKey){
					OrbView& orbView = *selectedOrb.orbView;
					setOrbSelectIndex(orbView, getPrevOrbIndex(*selectedOrb.orbUi, orbView.targetIndex));
				}

				auto level= overworldLevel();
				if (selectedOrb.selectKey && level.has_value()){
  		  	playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, std::nullopt, false);
  		  	goToLevel(level.value());
  		  	return;
  			}
			}
		}
	}
}




std::unordered_map<std::string, CutsceneOption> cutsceneDatas {
	{ "testorb", CutsceneOption {
			.text = { "I remember a nightmare I had as a child.\n\n"
"A large pyramid\n"
"moving slowly\n"
"on a tilted plane.\n\n"
"There was nothing.\n"
"And yet,\n"
"it terrified me more than anything else." },
			.rail = "cutscene1-rail",
			.letterbox = "Nothing to Be Afraid Of",
	}},
	{ "testorb3", CutsceneOption {
			.text = { "This is the first page of text", "This is second page of text", "This is third page of text" },
			.rail = "cutscene1-rail",
			.letterbox = "Abstract Geometry",
	}},
};

std::function<void(EasyCutscene&)> createCutscene(std::string option, std::optional<glm::vec3> position, bool skipAnimation){
	auto& cutsceneData = cutsceneDatas.at(option);
 	auto text = cutsceneData.text;
 	auto rail = cutsceneData.rail;
 	auto letterboxText = cutsceneData.letterbox;

 	bool hasAlreadyPlayed = cutsceneData.hasAlreadyPlayed;
 	cutsceneData.hasAlreadyPlayed = true;

	return [text, rail, letterboxText, position, skipAnimation, hasAlreadyPlayed](EasyCutscene& cutscene) -> void {
		int index = -1;
		auto getIndex = [&index]() -> int {
			index++;
			return index;
		};

  	if (initialize(cutscene)){
    	//glm::vec3 initialPos = glm::vec3(0.f, 10.f, 0.f);
    	auto cameraId = findObjByShortName(">menu-view", std::nullopt);
  		auto initialPos = position.has_value() ? position.value() : gameapi -> getGameObjectPos(cameraId.value(), true, "[gamelogic] - ballIntroOpening pos");
    	auto initialRot = gameapi -> getGameObjectRotation(cameraId.value(), true, "[gamelogic] - ballIntroOpening");

  		BallIntroData ballIntroData {
    		.showText = true,
    		.initialPos = initialPos,
    		.cameraId = cameraId.value(),
    		.railLengthMs = 0,
    	};

    	gameapi -> setGameObjectPosition(cameraId.value(), initialPos, true, Hint { .hint = "[gamelogic] - ballIntroOpening" });

			auto railId = railIdForName(rail);

			if (railId.has_value()){
				auto rail = railForId(railId.value());
			 	auto railTotalTimeMs = timeToTriggerIndex(*rail.value(), std::nullopt) * 1000;
			 	std::cout << "cutscene: total length: " << railTotalTimeMs << std::endl;

				std::cout << "cutscene rail: ";
				for (auto& time : rail.value() -> times){
					std::cout << time << " ";
				}
				ballIntroData.times = rail.value() -> times;
				ballIntroData.railLengthMs = railTotalTimeMs;

				std::cout << std::endl;
				addManagedRailMovement(cameraId.value(), railId.value(), initialPos, initialRot);
			}
    	store(cutscene, ballIntroData);
    	showLetterBox(letterboxText, 10.f);
  	}

  	BallIntroData* introData = getStorage<BallIntroData>(cutscene);
  	modassert(introData, "intro data is null");

  	if (finalize(cutscene)){
  		removeManagedRailMovement(introData -> cameraId);
  	  ballModeLevelSelect();
  	}

  	if (skipAnimation || hasAlreadyPlayed){
  		setCutsceneFinished(cutscene);
  	}

  	if (glfwGetKey(window, 'K')){
  		setCutsceneFinished(cutscene);
  	}
  
  	for (int i = 0; i < (text.size() + 1); i++){
  		auto textIndex = getIndex();
  		waitUntil(cutscene, textIndex, i * 5000);
  		if (i < text.size() && finished(cutscene, textIndex)){
	  		gameapi -> drawText(text.at(i), 0.f + 0.1f, i * -0.2f, 12, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  		}
  	}

	};
}

void ballModeNewGame(){
	activeLayer = 0;
	for (auto& [_, cutscene] : cutsceneDatas){
		cutscene.hasAlreadyPlayed = false;
	}

  resetProgress();
  setShowLiveMenu(false);
  playGameplayClipById(getManagedSounds().teleportObjId.value(), std::nullopt, std::nullopt, false);
  currentCutscene = playCutscene(createCutscene("testorb", glm::vec3(0.f, 10.f, 0.f), false), std::nullopt);
}


void ballModeLevelSelect(){
	std::cout << "ballModeLevelSelect" << std::endl;
  setShowLiveMenu(false);
  auto cameraId = findObjByShortName(">menu-view", std::nullopt);
  auto levelNavInfo = getLevelOrbInfo(cameraId.value());
	setCameraToOrbView(cameraId.value(), levelNavInfo.orbUi, levelNavInfo.maxCompletedIndex, 0.1f);
  showLetterBoxHold("Level Select", 0.f);
  setLifetimeObject(cameraId.value(), []() -> void { hideLetterBox(); }, "ball mode level select");
}

GameTypeInfo getBallIntroMode(){
	GameTypeInfo ballIntroMode = GameTypeInfo {
	  .gametypeName = "ball-intro",
	  .events = { "ball-intro" },
	  .createGametype = [](void* data) -> std::any {
			IntroModeOptions* modeOptions = static_cast<IntroModeOptions*>(data);
	    return *modeOptions; 
	  },
	  .onEvent = [](std::any& gametype, std::string& event, std::any& value) -> bool {
	   	return false;
	  },
	  .getDebugText = [](std::any& gametype) -> std::string {
	    return std::string("ball mode");
	  },
	  .getScoreInfo = [](std::any& gametype, float startTime) -> std::optional<GametypeData> { return std::nullopt; },
	  .onKey = [](std::any& gametype, int key, int scancode, int action, int mods) -> void {
	  },
	  .onFrame = [](std::any& gametype) -> void {
	  	bool shouldShowProgress = !getGlobalState().showLiveMenu;
	  	if (!shouldShowProgress){
	  		return;
	  	}
	  	IntroModeOptions* introMode = std::any_cast<IntroModeOptions>(&gametype);
	  	modassert(introMode, "introMode options");
	  	auto levelOrbInfo = getLevelOrbInfo(introMode -> cameraId);
	  	auto orbIndex = levelOrbInfo.orbIndex;
	  	auto orbUiPtr = orbUiByName(levelOrbInfo.orbUi);
	  	auto& orbUi = *orbUiPtr.value();

	  	std::vector<std::string> allLevels;

	  	for (int i = 0; i < orbUi.orbs.size(); i++){
	  		auto& orb = orbUi.orbs.at(i);
	  		auto isComplete = orb.getOrbProgress().complete;
	  		bool selected = orbIndex.has_value() &&  (orb.index == orbIndex.value());
        gameapi -> drawRect(0.95f, 0.75 + (-0.1 * i), 0.05f, 0.05f, false, glm::vec4(0.f, 0.f, 0.f, 0.8f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);	  		
        if (isComplete){
	        gameapi -> drawRect(0.95f, 0.75 + (-0.1 * i), 0.01f, 0.01f, false, glm::vec4(1.0f, 0.9216f, 0.2314f, 0.4f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);	  		
        }
        if (selected){
        	gameapi -> drawRect(0.95f + (0.05f * 0.5f), 0.75 + (-0.1 * i), 0.005f, 0.05f, false, glm::vec4(0.f, 0.f, 1.f, 0.9f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);	  		
        }

        allLevels.push_back(orb.level);
	  	}


	  	if (activeLayer != introMode -> activeLayer){
				bool fromOverworld = introMode -> activeLayer == (orbLayers.size() - 1);
				bool toOverworld = activeLayer == (orbLayers.size() - 1);

	  		introMode -> activeLayer = activeLayer;
	  		auto position = gameapi -> getGameObjectPos(introMode -> cameraId, true, "active layer get orb pos");

 				stopRotate(introMode -> cameraId);
				removeCameraFromOrbView(introMode -> cameraId);
				
				if (currentCutscene.has_value()){
					removeCutscene(currentCutscene.value(), true);
				}

				std::cout << "overworld from: " << fromOverworld << std::endl;
				std::cout << "overworld to: " << toOverworld << std::endl;

				currentCutscene = playCutscene(createCutscene("testorb3", position, toOverworld || fromOverworld), std::nullopt);
	  	}

	  	struct DescInfo {
	  		std::vector<std::string> mainInfos;
	  		std::vector<std::string> hubInfos;
	  		std::vector<std::string> levelInfos;
	  	};

	  	std::optional<std::string> levelName = isOverworld() ? std::optional<std::string>(std::nullopt) : overworldLevel();
	  	auto progressInfo = getProgressInfo(currentOverworldName(), levelName, allLevels);
	  	DescInfo descInfo {
	  		.mainInfos = {
	  			std::string("overworld: ") + (isOverworld() ? "true" : "false"),
	  			std::string("total gems: ") + std::to_string(progressInfo.gemCount) + " / " + std::to_string(progressInfo.totalGemCount),
	  		},
	  		.hubInfos = {
	  			std::string("world: ") + progressInfo.worldProgressInfo.currentWorld,
	  			std::string("completed: ") + std::to_string(progressInfo.completedLevels) + " / " + std::to_string(progressInfo.totalLevels),
	  			std::string("total gems: ") + std::to_string(progressInfo.worldProgressInfo.gemCount) + " / " + std::to_string(progressInfo.worldProgressInfo.totalGemCount),
	  		},
	  		.levelInfos = {},
	  	};

	  	if (progressInfo.level.has_value()){
  			std::string parTime = print(progressInfo.level.value().parTime, 2);
  			std::string bestTime = "n/a";
  			if(progressInfo.level.value().bestTime.has_value()){
  				bestTime = print(progressInfo.level.value().bestTime.value(), 2);
  			}
	  		descInfo.levelInfos = {
	  			std::string("current level: ") + levelName.value(),
	  			std::string("par time: ") + parTime + "s",
	  			std::string("best time: ") + bestTime + "s",
	  			std::string("total gems: ") + std::to_string(progressInfo.level.value().gemCount) + " / " + std::to_string(progressInfo.level.value().totalGemCount),
	  		};
	  	}

 			for (int i = 0; i < descInfo.hubInfos.size(); i++){
	 	  		gameapi -> drawText(descInfo.hubInfos.at(i), -0.9f, 0.7f - (i * 0.1f), 12, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	  	}
			for (int i = 0; i < descInfo.mainInfos.size(); i++){
	 	  		gameapi -> drawText(descInfo.mainInfos.at(i), -0.9f, -0.6f - (i * 0.1f), 12, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	  	}
	  	if (!isOverworld()){
	  		for (int i = 0; i < descInfo.levelInfos.size(); i++){
	 	  		gameapi -> drawText(descInfo.levelInfos.at(i), 0.6f, 0.7f - (i * 0.1f), 8, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	  		}	  		
	  	}

	  	std::cout << "ballintro mode frame" << std::endl;
	  },
	};
	return ballIntroMode;
}
