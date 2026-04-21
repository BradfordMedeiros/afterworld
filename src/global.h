#ifndef MOD_AFTERWORLD_GLOBAL
#define MOD_AFTERWORLD_GLOBAL

#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"
#include "./gameworld/save.h"

struct RouteState {
  bool paused;
  bool inGameMode;
  bool showMouse;
};

struct SystemConfig {
  bool showScreenspaceGrid = false;
  bool showConsole = false;
  bool showKeyboard = false;
};

struct ControlData {
  // Frame control data
  float xNdc = 0.f;
  float yNdc = 0.f;
  glm::vec2 mouseVelocity = glm::vec2(0.f, 0.f);

  glm::vec2 texCoordUv = glm::vec2(0.f, 0.f);
  glm::vec2 texCoordUvView = glm::vec2(0.f, 0.f);

  bool leftMouseDown = false;
  bool rightMouseDown = false;
  bool middleMouseDown = false;

  // User state
  std::optional<objid> selectedId;
  std::optional<objid> lookAtId;

  // Settings
  float xsensitivity = 1.f;
  float ysensitivity = 1.f;
  bool invertY = false;
};

struct GlobalState {
  SystemConfig systemConfig; 
  ControlData control;
  bool showEditor;
  bool isFreeCam;
  RouteState routeState;
  bool userRequestedPause;
};

bool disableGameInput();

bool isPaused();
void updateState();

GlobalState& getGlobalState();
void setShowEditor(bool shouldShowEditor);
void toggleKeyboard();

bool showConsole();
void setShowConsole(bool showConsole);

#endif 