#ifndef MOD_AFTERWORLD_COMPONENTS_INDEX
#define MOD_AFTERWORLD_COMPONENTS_INDEX

#include <string>
#include <optional>
#include "../util.h"

struct RouterHistory {
  float currentRouteTime;
  std::deque<std::string> history;
  std::optional<std::any> data;
  std::optional<bool> routeChangedForceReload;
};
RouterHistory createHistory();
void pushHistory(RouterHistory& history, std::vector<std::string> path, bool replace, std::optional<std::any> data = std::optional<std::any>(std::nullopt), bool forceLoad = false);
void popHistory(RouterHistory& history);

std::string fullHistoryStr(RouterHistory& history);

struct PathMatch {
  bool matches;
  std::vector<std::string> params;
};

// can insert * instead of the subpath and that will match anything
PathMatch matchPath(std::string path, std::string expression);


struct UiStateContext {
  RouterHistory* routerHistory;
};
void pushHistory(std::vector<std::string> route, bool replace, std::optional<std::any> data = std::optional<std::any>(std::nullopt), bool forceLoad = false);
void popHistory();

struct UiModeNone{};
struct FpsModeUi {};

struct BallLevelComplete {};
struct BallComponentOptions {
  std::optional<std::function<float()>> elapsedTime;
  bool showElapsedTime = false;

  std::optional<BallLevelComplete> levelComplete;

  bool showPowerup = false;
  std::optional<std::string> powerupTexture;
  std::optional<float> powerupStartTime;
  std::optional<float> powerupDuration;
};

struct BallLevelSelectInfo {
  std::string world;
  std::string level;
  std::string parTime;
  std::string bestTime;
  int gems = 0;
  int totalGems = 0;
};
struct BallModeUi {
  BallComponentOptions ballMode;
  std::optional<BallLevelSelectInfo> levelSelect;
};

struct MainMenu2Options {
  glm::vec4 backgroundColor;
  float offsetY;

  bool showSettings = false;
  bool canContinue = false;
  std::function<void()> onNewGame = []() -> void {};
  std::function<void()> onContinueGame = []() -> void {};
};

struct BallInfo {};
struct LiveMenu {
  MainMenu2Options options;
  std::optional<BallInfo> ballInfo;
  std::string text;
};
struct GameOverUi {};

typedef std::variant<UiModeNone, FpsModeUi, BallModeUi, LiveMenu, GameOverUi> UiMode;
void changeUiMode(UiMode);

std::optional<BallModeUi*> getBallModeUI();
std::optional<LiveMenu*> getLiveMenuUi();


struct TerminalImage {
  std::string image;
};
struct TerminalImageLeftTextRight {
  std::string image;
  std::string text;
};
struct TerminalText {
  std::string text;
};
typedef std::variant<TerminalImage, TerminalImageLeftTextRight, TerminalText> TerminalDisplayType;
struct TerminalConfig {
  float time;
  TerminalDisplayType terminalDisplay;
};


void setTerminalConfig(std::optional<TerminalConfig> terminalConfig);
std::optional<TerminalConfig*> getTerminalConfig();



#endif

