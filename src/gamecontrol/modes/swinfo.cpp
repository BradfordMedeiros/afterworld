#include "./swinfo.h"

extern CustomApiBindings* gameapi;
extern GameTypes gametypeSystem;

void showLetterBoxHold(std::string title, float fadeInTime);
void setPauseMenuOverride(std::optional<std::function<void()>> goToMenuFn);
void inputOverride(bool paused, bool showMouse, bool hidePauseMenu);

void drawImGuiText(std::string text, std::optional<glm::vec2> positionNdi);
void setCanTogglePause(bool canPause);
void goToLevel(std::string levelShortName, std::optional<std::any> hint, bool forceReload);

std::vector<std::string> gatherSwInfo(){
  return {
    "Software Version: Some Version",
    "Some Software Notes, other stuff",
    "This is some text\nWowverycool",
  };
}

GameTypeInfo simpleUi {
  .gametypeName = "simpleui",
  .createGametype = [](void* data) -> std::any {
    showLetterBoxHold("Software Information", 0.f);

    setPauseMenuOverride([]() -> void {
       goToLevel("ballselect", std::nullopt, true);
    });
    setCanTogglePause(true);

  	return NULL;
  },
  .onEvent = [](std::any& gametype, std::string& event, std::any& value) -> void {},
  .onFrame = [](std::any& gametype) -> void {
    if (isPaused()){
      return;
    }
    auto infos = gatherSwInfo();
    for (int i = 0; i < infos.size(); i++){
      drawImGuiText(infos.at(i), glm::vec2(0.5f, 0.5f + (i * -0.1f)));
    }
  },
};

void startSimpleUiMode(objid sceneId){
	changeGameType(gametypeSystem, simpleUi, "simpleui", NULL);
}
