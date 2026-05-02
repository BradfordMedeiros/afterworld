#include "./intro.h"

extern CustomApiBindings* gameapi;
extern GameTypes gametypeSystem;

void goToLevel(std::string levelShortName);
void ballModeLevelSelect();
void stopRotate(objid id);
void setLifetimeObject(objid id, std::function<void()> fn, std::string hint);
void inputOverride(bool paused, bool showMouse);
void setPauseMenuOverride(std::optional<std::function<void()>> goToMenuFn);

std::optional<objid> currentCutscene;
int activeLayer = 0;
std::string activeWorldName;
std::optional<objid> orbCameraId;

struct IntroModeOptions {
   objid cameraId;
   int activeLayer;
};

GameTypeInfo getBallIntroMode();

void ballModeNewGame();
void ballModeLevelSelect();


void startIntroMode(objid sceneId){	
	setPauseMenuOverride([]() -> void {
    goToLevel("ballselect");
	});

	if (currentCutscene.has_value()){
		removeCutscene(currentCutscene.value(), true);
		currentCutscene = std::nullopt;
	}
  auto cameraId = findObjByShortName(">menu-view", sceneId);
	removeCameraFromOrbView(cameraId.value());
  setTempCamera(cameraId.value(), 0);

  inputOverride(false, true);
  changeUiMode(LiveMenu {
   	.options = MainMenu2Options {
   		.backgroundColor = glm::vec4(1.f, 0.f, 0.f, 1.f),
   		.offsetY = 0.f,
			.onNewGame = []() -> void {
				ballModeNewGame();
			},
			.onContinueGame = []() -> void {
				ballModeLevelSelect();
			},
   	},
  });

  showLetterBoxHold("", 0.f);
  orbCameraId = cameraId;
  IntroModeOptions modeOptions {
  	.cameraId = cameraId.value(),
  };
  auto ballIntro = getBallIntroMode();
	changeGameType(gametypeSystem, ballIntro, "ball-intro", &modeOptions);
}

void endIntroMode(){
  if (currentCutscene.has_value()){
    removeCutscene(currentCutscene.value());
  }
}


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

bool isOverworld(){
	return activeLayer == (orbLayers.size() - 1);
}

void goToOverWorld(){
	activeWorldName = orbLayers.at(activeLayer).orbUi;
	activeLayer = orbLayers.size() - 1;
}

struct LevelOrbNavInfo {
	std::string orbUi;
	std::optional<int> orbIndex;
	std::optional<int> maxCompletedIndex;
};

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
	  	//bool shouldShowProgress = !getGlobalState().showLiveMenu;
	  	//if (!shouldShowProgress){
	  	//	return;
	  	//}
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
				currentCutscene = playCutscene(simpleNarratedMovement("testorb3", position, toOverworld || fromOverworld, ballModeLevelSelect), std::nullopt);
	  	}

	  	struct DescInfo {
	  		std::vector<std::string> mainInfos;
	  		std::vector<std::string> hubInfos;
	  		std::vector<std::string> levelInfos;
	  	};

	  	std::optional<std::string> levelName = isOverworld() ? std::optional<std::string>(std::nullopt) : overworldLevel();
	  	auto progressInfo = getProgressInfo(activeWorldName, levelName, allLevels);
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

void ballModeNewGame(){
  activeLayer = 0;
  for (auto& [_, cutscene] : cutsceneDatas){
  	cutscene.hasAlreadyPlayed = false;
  }

  resetProgress();
  playGameplayClipById(getManagedSounds().teleportObjId.value(), std::nullopt, std::nullopt, false);

  inputOverride(false, false);
	changeUiMode(UiModeNone{});

  currentCutscene = playCutscene(simpleNarratedMovement("testorb", glm::vec3(0.f, 10.f, 0.f), false, ballModeLevelSelect), std::nullopt);
}

void ballModeLevelSelect(){
	std::cout << "ballModeLevelSelect" << std::endl;
  inputOverride(false, false);
	changeUiMode(UiModeNone{});

  auto cameraId = findObjByShortName(">menu-view", std::nullopt);
  auto levelNavInfo = getLevelOrbInfo(cameraId.value());
	setCameraToOrbView(cameraId.value(), levelNavInfo.orbUi, levelNavInfo.maxCompletedIndex, 0.1f);
  showLetterBoxHold("Level Select", 0.f);
  setLifetimeObject(cameraId.value(), []() -> void { hideLetterBox(); }, "ball mode level select");
}

