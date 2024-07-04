#ifndef MOD_AFTERWORLD_GLOBAL
#define MOD_AFTERWORLD_GLOBAL

#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"

struct RouteState {
  bool paused;
  bool inGameMode;
  bool showMouse;
};

struct GlobalState {
  bool showEditor;
  bool showScreenspaceGrid;
  bool showConsole;
  bool showKeyboard;

  bool showGameHud;

  bool showTerminal;
  float lastToggleTerminalTime;

  float xNdc;
  float yNdc;
  glm::vec2 texCoordUv;
  glm::vec2 texCoordUvView;
  std::optional<objid> selectedId;

  bool godMode;


  float xsensitivity;
  float ysensitivity;
  bool invertY;
  bool disableGameInput;

  bool leftMouseDown;
  bool rightMouseDown;
  bool middleMouseDown;

  RouteState routeState;

  glm::vec3 playerVelocity;
};

void setPaused(bool paused);
bool isPaused();
void enterGameMode();
void exitGameMode();

void setRouterGameState(RouteState routeState);

GlobalState& getGlobalState();
void initGlobal();
void setShowEditor(bool shouldShowEditor);
void toggleKeyboard();

bool leftMouseDown();
bool rightMouseDown();
bool middleMouseDown();

bool showConsole();
void setShowConsole(bool showConsole);
void setShowTerminal(bool showTerminal);

void setPlayerVelocity(glm::vec3 velocity);
glm::vec3 getPlayerVelocity();

std::vector<std::vector<std::string>> debugPrintGlobal();

#endif 