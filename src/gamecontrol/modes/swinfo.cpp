#include "./swinfo.h"

extern CustomApiBindings* gameapi;
extern GameTypes gametypeSystem;

void showLetterBoxHold(std::string title, float fadeInTime);
void setPauseMenuOverride(std::optional<std::function<void()>> goToMenuFn);
void inputOverride(bool paused, bool showMouse, bool hidePauseMenu);

void drawImGuiText(std::string text, std::optional<glm::vec2> positionNdi);
void setCanTogglePause(bool canPause);
void goToLevel(std::string levelShortName, std::optional<std::any> hint, bool forceReload);


struct SWInfoModeOptions {
  std::string option;
};
std::optional<SWInfoModeOptions> swInfoModeOptions;
std::optional<SWInfoModeOptions> getSwModeOptions(){
  return swInfoModeOptions;
}

GameTypeInfo simpleUi {
  .gametypeName = "simpleui",
  .createGametype = [](void* data) -> std::any {
    SWInfoModeOptions* modeOptionsPtr = static_cast<SWInfoModeOptions*>(data);
    swInfoModeOptions = *modeOptionsPtr;

    if (swInfoModeOptions.value().option == "swinfo"){
      showLetterBoxHold("Software Information", 0.f);
    }else if (swInfoModeOptions.value().option == "credits"){
      showLetterBoxHold("Credits", 0.f);
    }else if (swInfoModeOptions.value().option == "stats"){
      showLetterBoxHold("Statistics", 0.f);
    }
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

    auto modeOption = getSwModeOptions();
    if (!modeOption.has_value()){
      return;
    }
    if (modeOption.value().option == "swinfo"){
      struct SoftwareLib {
        std::string name;
        std::optional<std::string> extraText;
      };
      struct SoftwareInfo {
        std::string build;
        std::vector<SoftwareLib> libs; 
      };
      SoftwareInfo softwareInfo{
        .build = "commit: Some build number",
        .libs = {
          SoftwareLib { .name = "example library 1", .extraText = "this is some extra library text 1" },
          SoftwareLib { .name = "example library 2", .extraText = "this is some extra library text 2" },
          SoftwareLib { .name = "example library 3", .extraText = "this is some extra library text 3" },
        }
      };

      std::vector<std::string> infos { "Software Information" };
      infos.push_back("Build: " + softwareInfo.build);
      for (auto& lib : softwareInfo.libs){
        infos.push_back("Lib: " + lib.name);
        if (lib.extraText.has_value()){
          infos.push_back("                              " + lib.extraText.value());
        }
      }

      for (int i = 0; i < infos.size(); i++){
        drawImGuiText(infos.at(i), glm::vec2(0.5f, 0.7f + (i * -0.05f)));
      }
    }else if (modeOption.value().option == "credits"){
      std::vector<std::string> infos {
          "Credits:",
        "Programming / Art / Game Design:  Bradford Medeiros",
        "Additional Levels:  Samantha Wang",
      };
      for (int i = 0; i < infos.size(); i++){
        drawImGuiText(infos.at(i), glm::vec2(0.5f, 0.7f + (i * -0.05f)));
      }
    }else if (swInfoModeOptions.value().option == "stats"){
      struct PlayerDisplayStats {
        int lostSouls = -1;
        int soulsDelivered = -1;
        int numberOfJumps = -1;
      };
      PlayerDisplayStats playerDisplayStats{};
      std::vector<std::string> infos {
          "Player statistics",
      };
      infos.push_back("Lost Souls: " + std::to_string(playerDisplayStats.lostSouls));
      infos.push_back("Souls Delivered: " + std::to_string(playerDisplayStats.soulsDelivered));
      infos.push_back("Number of Jumps: " +  std::to_string(playerDisplayStats.numberOfJumps));

      for (int i = 0; i < infos.size(); i++){
        drawImGuiText(infos.at(i), glm::vec2(0.5f, 0.7f + (i * -0.05f)));
      }
    }



  },
};

SWInfoModeOptions sWInfoModeOptions{};
void startSimpleUiMode(objid sceneId, std::optional<std::string> modeOption){
  if (!modeOption.has_value()){
    return;
  }

  sWInfoModeOptions = SWInfoModeOptions {
    .option = modeOption.value(),
  };
	changeGameType(gametypeSystem, simpleUi, "simpleui", &sWInfoModeOptions);
}
