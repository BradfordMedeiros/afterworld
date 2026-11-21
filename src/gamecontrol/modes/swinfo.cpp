#include "./swinfo.h"
#include "../../ui/mainui.h"

extern GameTypes gametypeSystem;

void showLetterBoxHold(std::string title, float fadeInTime);
void setPauseMenuOverride(std::optional<std::function<void()>> goToMenuFn);
void inputOverride(bool paused, bool showMouse, bool hidePauseMenu);

void setCanTogglePause(bool canPause);
void goToLevel(std::string levelShortName, std::optional<std::any> hint, bool forceReload);


struct SWInfoModeOptions {
  std::string option;
};

GameTypeInfo simpleUi {
  .gametypeName = "simpleui",
  .createGametype = [](void* data) -> std::any {
    SWInfoModeOptions* modeOptionsPtr = static_cast<SWInfoModeOptions*>(data);

    if (modeOptionsPtr->option == "swinfo"){
      showLetterBoxHold("Software Information", 0.f);
    }else if (modeOptionsPtr->option == "credits"){
      showLetterBoxHold("Credits", 0.f);
    }else if (modeOptionsPtr->option == "stats"){
      showLetterBoxHold("Statistics", 0.f);
    }
    setPauseMenuOverride([]() -> void {
       goToLevel("ballselect", std::nullopt, true);
    });
    setCanTogglePause(true);

  	return NULL;
  },
  .onEvent = [](std::any& gametype, std::string& event, std::any& value) -> void {},
  .onFrame = [](std::any& gametype) -> void {},
};

SWInfoModeOptions sWInfoModeOptions{};
void startSimpleUiMode(objid sceneId, std::optional<std::string> modeOption){
  if (!modeOption.has_value()){
    return;
  }

  sWInfoModeOptions = SWInfoModeOptions {
    .option = modeOption.value(),
  };

  SimpleTextDisplayUi textDisplay;
  if (modeOption.value() == "swinfo"){
    textDisplay.header = "Software Information";
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

    textDisplay.rows.push_back(SimpleTextDisplayColumnRow{
      .left = "Build",
      .right = softwareInfo.build,
    });
    for (const auto& lib : softwareInfo.libs){
      textDisplay.rows.push_back(SimpleTextDisplayColumnRow{
        .left = lib.name,
        .right = lib.extraText.value_or(""),
      });
    }
  }else if (modeOption.value() == "credits"){
    textDisplay.header = "Credits";
    textDisplay.rows = {
      SimpleTextDisplayTextRow{ .text = "Programming / Art / Game Design: Bradford Medeiros" },
      SimpleTextDisplayTextRow{ .text = "Additional Levels: Samantha Wang" },
    };
  }else if (modeOption.value() == "stats"){
    struct PlayerDisplayStats {
      int lostSouls = -1;
      int soulsDelivered = -1;
      int numberOfJumps = -1;
    };
    PlayerDisplayStats playerDisplayStats{};

    textDisplay.header = "Player Statistics";
    textDisplay.rows = {
      SimpleTextDisplayColumnRow { .left = "Lost Souls", .right = std::to_string(playerDisplayStats.lostSouls) },
      SimpleTextDisplayColumnRow { .left = "Souls Delivered", .right = std::to_string(playerDisplayStats.soulsDelivered) },
      SimpleTextDisplayColumnRow { .left = "Number of Jumps", .right = std::to_string(playerDisplayStats.numberOfJumps) },
    };
  }else{
    textDisplay.header = "Information";
    textDisplay.rows.push_back(SimpleTextDisplayTextRow{ .text = modeOption.value() });
  }

  changeUiMode(textDisplay);
	changeGameType(gametypeSystem, simpleUi, "simpleui", &sWInfoModeOptions);
}
