#include "./intro.h"

extern CustomApiBindings* gameapi;
extern GameTypes gametypeSystem;

void goToLevel(std::string levelShortName);
void stopRotate(objid id);
void setLifetimeObject(objid id, std::function<void()> fn, std::string hint);
void inputOverride(bool paused, bool showMouse);
void setPauseMenuOverride(std::optional<std::function<void()>> goToMenuFn);


void setToMultiOrbView(objid cameraId);
bool isOverworld(MultiOrbView& multiOrbView);
std::optional<OrbUi*> currentMultiOrbUi(MultiOrbView& multiOrbView, std::optional<int>* _orbIndex);
std::optional<std::string> getSelectedLevel(MultiOrbView& multiOrbView);

std::optional<objid> currentCutscene;


struct IntroModeOptions {
   objid cameraId;
   int activeLayer;
};


extern std::optional<MultiOrbView> multiOrbViewOpt;


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
	  	if (!multiOrbViewOpt.has_value()){
	  		return;
	  	}
	  	MultiOrbView& multiOrbView = multiOrbViewOpt.value();

	  	IntroModeOptions* introMode = std::any_cast<IntroModeOptions>(&gametype);
	  	modassert(introMode, "introMode options");

	  	std::optional<int> orbIndex = 0;
	  	auto& orbUi = *currentMultiOrbUi(multiOrbViewOpt.value(), &orbIndex).value();

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

	  	bool layerChanged = multiOrbView.activeLayer != introMode -> activeLayer;
			bool fromOverworld = introMode -> activeLayer == (multiOrbView.orbLayers.size() - 1);
			bool toOverworld = multiOrbView.activeLayer == (multiOrbView.orbLayers.size() - 1);
	  	if (layerChanged){
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
  		introMode -> activeLayer = multiOrbView.activeLayer;


	  	struct DescInfo {
	  		std::vector<std::string> mainInfos;
	  		std::vector<std::string> hubInfos;
	  		std::vector<std::string> levelInfos;
	  	};

	  	std::optional<std::string> levelName = getSelectedLevel(multiOrbView);
	  	auto progressInfo = getProgressInfo(multiOrbView.activeWorldName, levelName, allLevels);
	  	DescInfo descInfo {
	  		.mainInfos = {
	  			std::string("overworld: ") + (isOverworld(multiOrbView) ? "true" : "false"),
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
	  	if (!isOverworld(multiOrbView)){
	  		for (int i = 0; i < descInfo.levelInfos.size(); i++){
	 		 		gameapi -> drawText(descInfo.levelInfos.at(i), 0.6f, 0.7f - (i * 0.1f), 8, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
	  		}	  		
	  	}
	  },
	};
	return ballIntroMode;
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



