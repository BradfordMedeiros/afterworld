#include "./intro.h"

extern CustomApiBindings* gameapi;
extern GameTypes gametypeSystem;

void goToLevel(std::string levelShortName);
void setLifetimeObject(objid id, std::function<void()> fn, std::string hint);
void inputOverride(bool paused, bool showMouse);
void setPauseMenuOverride(std::optional<std::function<void()>> goToMenuFn);

std::optional<objid> currentCutscene;

struct IntroModeOptions {
   objid cameraId;
};


struct DescInfo {
	std::vector<std::string> mainInfos;
	std::vector<std::string> hubInfos;
	std::vector<std::string> levelInfos;
	std::vector<WorldOrbInfos> worldOrbInfos;
	bool onOverworld;
};

DescInfo getDescriptionInfo(MultiOrbView& multiOrbView){
	bool onOverworld = isOverworld(multiOrbView);

	std::string worldName = "test";  // multiOrbView.activeWorldName
	std::optional<std::string> levelName = getSelectedLevel(multiOrbView);


	auto progressInfo = getPlaylistProgressInfo();
	auto worldProgressInfo = getWorldProgressInfo(worldName);
	DescInfo descInfo {
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

void drawDescInfo(DescInfo& descInfo){
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

GameTypeInfo getBallIntroMode(){
	GameTypeInfo ballIntroMode = GameTypeInfo {
	  .gametypeName = "ball-intro",
	  .createGametype = [](void* data) -> std::any {
			IntroModeOptions* modeOptions = static_cast<IntroModeOptions*>(data);
	    return *modeOptions; 
	  },
	  .onEvent = [](std::any& gametype, std::string& event, std::any& value) -> void {},
	  .onKey = [](std::any& gametype, int key, int scancode, int action, int mods) -> void {},
	  .onFrame = [](std::any& gametype) -> void {
	  	IntroModeOptions* introMode = std::any_cast<IntroModeOptions>(&gametype);
	  	modassert(introMode, "introMode options");
	  	auto multiOrbViewPtr = multiorbViewByCamera(introMode -> cameraId);
	  	if (!multiOrbViewPtr.has_value()){
	  		return;
	  	}
	   	MultiOrbView& multiOrbView = *multiOrbViewPtr.value();
	  	auto descInfo = getDescriptionInfo(multiOrbView);
	  	drawDescInfo(descInfo);
	  },
	};
	return ballIntroMode;
}

void ballModeLevelSelect(){
	std::cout << "ballModeLevelSelect" << std::endl;

  inputOverride(false, false);
	changeUiMode(UiModeNone{});
  showLetterBoxHold("Level Select", 0.f);

  auto cameraId = findObjByShortName(">menu-view", std::nullopt);
  setLifetimeObject(cameraId.value(), []() -> void { hideLetterBox(); }, "ball mode level select");

  setToMultiOrbView(cameraId.value());
}
void ballModeNewGame(){
  for (auto& [_, cutscene] : cutsceneDatas){
  	cutscene.hasAlreadyPlayed = false;
  }

  resetProgress();
  playGameplayClipById(getManagedSounds().teleportObjId.value(), std::nullopt, std::nullopt, false);

  inputOverride(false, false);
	changeUiMode(UiModeNone{});

  currentCutscene = playCutscene(simpleNarratedMovement("testorb", glm::vec3(0.f, 10.f, 0.f), false, ballModeLevelSelect), std::nullopt);
}

void startIntroMode(objid sceneId){	
	setPauseMenuOverride([]() -> void {
    goToLevel("ballselect");
	});

	if (currentCutscene.has_value()){
		removeCutscene(currentCutscene.value(), true);
		currentCutscene = std::nullopt;
	}
  auto cameraId = findObjByShortName(">menu-view", sceneId);
	removeCameraFromMultiOrbView(cameraId.value());
  setTempCamera(cameraId.value(), 0);
  setToMultiOrbView(cameraId.value());

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



