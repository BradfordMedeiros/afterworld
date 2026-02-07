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

void startRotate(objid id);
void stopRotate(objid id);

bool inBallMode = false;

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
	  playGameplayClipById(getManagedSounds().teleportObjId.value(), std::nullopt, std::nullopt);
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
  	ballModeLevelSelect(true);
  });
}

void setBallLevelComplete(){
	if (!inBallMode){
		return;
	}
	inBallMode = false;
	std::cout << "set ball level complete: " << activeLevel.value() << std::endl;
	changeGameType(gametypeSystem, NULL, NULL);
	markLevelComplete(activeLevel.value(), true);
	playCutscene(ballEndGameplay, std::nullopt);	
}

void createBallObj(objid sceneId, glm::vec3 position){
  GameobjAttributes attr { 
  	.attr = {
  		{ "vehicle", "ball" },
			{ "texture", "./res/textures/wood.jpg" },
			{ "tint", glm::vec4(1.f, 1.f, 1.f, 0.1f) },
			{ "physics_restitution", 0.5f },
			{ "mesh", "../gameresources/build/primitives/sphere.gltf" },
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

void showTimeElapsed(std::optional<float> startTime){
		if (!showBallOptions().has_value()){
			return;
		}
		auto ballOptions = showBallOptions().value();
		ballOptions.startTime = startTime;
		setShowBallOptions(ballOptions);
}

void setPowerupTexture(std::string texture){
	auto ballOptions = showBallOptions();
	if (!ballOptions.has_value()){
		return;
	}
	if (ballOptions.has_value()){
		auto newBallOptions = ballOptions.value();
		newBallOptions.powerupTexture = texture;;
		setShowBallOptions(newBallOptions);
	}
}

std::optional<BallPowerup> getPowerup(){
	auto activePlayer = getActivePlayerId(0);
	auto vehicleIds = getVehicleIds();
	if (vehicleIds.size() == 0){
		return std::nullopt;
	}
	return getBallPowerup(vehicles, vehicleIds.at(0));
} 

void startBallMode(objid sceneId){
	inBallMode = true;
	auto playerSpawnId = findObjByShortName("playerspawn", std::nullopt);
	auto position = gameapi -> getGameObjectPos(playerSpawnId.value(), true, "[gamelogic] ball - get playerspawn position");
	createBallObj(sceneId, position);
	BallModeOptions modeOptions {};
	changeGameType(gametypeSystem, "ball", &modeOptions);
	setHudEnabled(false);
}

void endBallMode(){
	inBallMode = false;
	setCanExitVehicle(true);
	setShowBallOptions(std::nullopt);
	changeGameType(gametypeSystem, NULL, NULL);
	setHudEnabled(true);
}

GameTypeInfo getBallMode(){
	GameTypeInfo ballMode = GameTypeInfo {
	  .gametypeName = "ball",
	  .events = { "ball" },
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
 	   	showTimeElapsed(gameapi -> timeSeconds(true));

			changeUi(true);

	    return *modeOptions; 
	  },
	  .onEvent = [](std::any& gametype, std::string& event, std::any& value) -> bool {
	  	BallModeOptions* ballMode = std::any_cast<BallModeOptions>(&gametype);
	  	modassert(ballMode, "ballMode options");

	  	std::string* message = std::any_cast<std::string>(&value);
	  	modassert(message, "invalid type ball-mode");
	  	std::cout << "from ball mode: " << event << ", " << *message << std::endl;

	  	if (*message == "reset"){
	  		modassert(ballMode -> initialBallPos.has_value(), "no initial ball position");
	  		gameapi -> setGameObjectPosition(ballMode -> ballId.value(), ballMode -> initialBallPos.value(), true, Hint { .hint = "[gamelogic] - ball set pos" });
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
	  		gameapi -> sendNotifyMessage("ball-game", std::string("reset"));
	  	}
	  	std::cout << "ball mode: " << key << ", " << action << std::endl;
	  },
	  .onFrame = [](std::any& gametype) -> void {
	  	BallModeOptions* ballMode = std::any_cast<BallModeOptions>(&gametype);
	  	modassert(ballMode, "ballMode options");
	  	std::cout << "ball onframe" << std::endl;
	  	if (ballMode -> ballId.has_value()){
	  		auto powerup = getPowerup();
	  		if (powerup.has_value()){
		  		std::cout << "powerup: " << print(powerup.value()) << std::endl;
	  			if (powerup.value() == BIG_JUMP){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/jump.png");
	 		  		return;
	  			}else if (powerup.value() == LAUNCH_FORWARD){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/dash.png");
	 		  		return;
	  			}else if (powerup.value() == LOW_GRAVITY){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/bounce.png");
	 		  		return;
	  			}else if (powerup.value() == REVERSE_GRAVITY){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/bounce.normal.png");
	 		  		return;
	  			}else if (powerup.value() == TELEPORT){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/teleport.png");
	 		  		return;
	  			}else{
	  				modassert(false, "invalid powerup size ball.cpp");
	  			}
	  		}
	  	}

  		setPowerupTexture("../gameresources/build/textures/ballgame/none.png");
	  },
	};
	return ballMode;
}

/////////////////////////////////////////////////////////
struct IntroModeOptions {
   objid cameraId;
   int activeLayer;
};

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

  IntroModeOptions modeOptions {
  	.cameraId = cameraId.value(),
  	.activeLayer = 0,
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

static int activeLayer = 0;
static std::optional<int> activeWorld = std::nullopt;
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
	if (!activeWorld.has_value()){
		return orbLayers.at(0).orbUi;
	}
  auto& orbUi = *orbUiByName("metaworld").value();
  for (int i = 0; i < orbUi.orbs.size(); i++){
  	if (activeWorld.value() == orbUi.orbs.at(i).index){
  		return orbUi.orbs.at(i).level;
  	}
  }
  modassert(false, "should not have reached here currentOverworldName");
	return "unknown";
}
void goToOverWorld(){
	auto& activeLayerName = orbLayers.at(activeLayer).orbUi;
  auto orbUi = orbUiByName(orbLayers.at(orbLayers.size() - 1).orbUi);
  auto &orbs = orbUi.value() -> orbs;

  bool foundOrb = false;
  for (int i = 0; i < orbs.size(); i++){
  	if (activeLayerName == orbs.at(i).level){
  		activeWorld = orbs.at(i).index;
  		std::cout << "goToOverWorld found orb: " << activeLayerName << ", activeworld: " << (activeWorld.has_value() ? activeWorld.value() : -1) << std::endl;
  		foundOrb = true;
 			break;
  	}
  }

  if (!foundOrb){
  	activeWorld = std::nullopt;
  }
	activeLayer = orbLayers.size() - 1;
}

LevelOrbNavInfo getLevelOrbInfo(objid cameraId){
	auto& orbLayer = orbLayers.at(activeLayer);

	std::string orbUiName = orbLayer.orbUi;
  auto orbUi = orbUiByName(orbUiName);
  auto maxCompletedIndex = getMaxCompleteOrbIndex(*orbUi.value());
 	auto orbIndex = getActiveOrbViewIndex(cameraId);
	return LevelOrbNavInfo {
		.orbUi = orbUiName,
		.orbIndex = orbIndex,
		.maxCompletedIndex = orbUiName == "metaworld" ? activeWorld : maxCompletedIndex,
	};
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
  	  ballModeLevelSelect(true);
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
  playGameplayClipById(getManagedSounds().teleportObjId.value(), std::nullopt, std::nullopt);
  currentCutscene = playCutscene(createCutscene("testorb", glm::vec3(0.f, 10.f, 0.f), false), std::nullopt);
}

void onModeOrbSelect(OrbSelection& selectedOrb){
	if (selectedOrb.moveRightKey && selectedOrb.orbView.has_value()){
		OrbView& orbView = *selectedOrb.orbView.value();
		OrbUi& orbUi = *selectedOrb.orbUi.value();
		setOrbSelectIndex(orbView, getNextOrbIndex(orbUi, orbView.targetIndex));
	}else if (selectedOrb.moveLeftKey && selectedOrb.orbView.has_value()){
		OrbView& orbView = *selectedOrb.orbView.value();
		OrbUi& orbUi = *selectedOrb.orbUi.value();
		setOrbSelectIndex(orbView, getPrevOrbIndex(orbUi, orbView.targetIndex));
	}
	

	if (selectedOrb.selectedOrb.has_value() && activeLayer == orbLayers.size() - 1){
		for (int i = 0; i < orbLayers.size() - 1; i++){
			if (orbLayers.at(i).orbUi == selectedOrb.selectedOrb.value() -> level){
				activeLayer = i;
			}
		}
		return;
	}

  if (selectedOrb.selectedOrb.has_value()){
    std::cout << "handleOrbViews orb: " << print(*selectedOrb.selectedOrb.value()) << std::endl;
    playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, std::nullopt);
    goToLevel(selectedOrb.selectedOrb.value() -> level);
    return;
  }
  if (selectedOrb.moveRight){
    goToOverWorld();
  }else if (selectedOrb.moveLeft){
    goToOverWorld();
  }
}


void ballModeLevelSelect(bool setOrbView){
	std::cout << "ballModeLevelSelect" << std::endl;
  setShowLiveMenu(false);
  auto cameraId = findObjByShortName(">menu-view", std::nullopt);
  auto levelNavInfo = getLevelOrbInfo(cameraId.value());
  if (setOrbView){
	  setCameraToOrbView(cameraId.value(), levelNavInfo.orbUi, levelNavInfo.maxCompletedIndex, 0.1f);
  }
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
	  	IntroModeOptions* introMode = std::any_cast<IntroModeOptions>(&gametype);
	  	modassert(introMode, "introMode options onKey");

			if (key == 'O'){
	  		goToOverWorld();
	  	}
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
	  	}

	  	if (activeLayer != introMode -> activeLayer){
				bool fromOverworld = introMode -> activeLayer == (orbLayers.size() - 1);
				bool toOverworld = activeLayer == (orbLayers.size() - 1);

	  		introMode -> activeLayer = activeLayer;
	  		auto position = gameapi -> getGameObjectPos(introMode -> cameraId, true, "active layer get orb pos");
	  		auto rotation = gameapi -> getGameObjectRotation(introMode -> cameraId, true, "active layer get orb rot");
 				std::cout << "handleOrbViews 2 set position: " << gameapi -> getGameObjNameForId(introMode -> cameraId).value() << ", to : " << print(position) << std::endl;

 				stopRotate(introMode -> cameraId);
				removeCameraFromOrbView(introMode -> cameraId);
				
	  		auto activeOrbUi = orbLayers.at(activeLayer).orbUi;

				if (currentCutscene.has_value()){
					removeCutscene(currentCutscene.value(), true);
				}

				std::cout << "overworld from: " << fromOverworld << std::endl;
				std::cout << "overworld to: " << toOverworld << std::endl;

				currentCutscene = playCutscene(createCutscene("testorb3", position, toOverworld || fromOverworld), std::nullopt);
	  	}

	  	gameapi -> drawText(std::string("world: ") + currentOverworldName(), -0.8f, 0.5f, 12, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	  	
	  	std::string numberOfLevels = "number gems: "; 
	  	gameapi -> drawText(std::string(numberOfLevels) + std::to_string(12) + " / " + std::to_string(20), -0.8f, 0.4f, 12, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	  	
	  	std::string parTime = "par time: 120s";
	  	gameapi -> drawText(parTime, -0.8f, 0.3f, 12, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);

	  	std::string bestTime = "best time: 13.34s";
	  	gameapi -> drawText(bestTime, -0.8f, 0.2f, 12, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);


	  	std::cout << "ballintro mode frame" << std::endl;
	  },
	};
	return ballIntroMode;
}
