#ifndef MOD_AFTERWORLD_GLOBAL
#define MOD_AFTERWORLD_GLOBAL

#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"

struct GlobalState {
  bool paused;
  bool inGameMode;
  bool showEditor;
  bool showScreenspaceGrid;
  bool showConsole;

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
};

void updateShowMouse(bool showMouse);
void setPaused(bool paused);
bool isPaused();
void enterGameMode();
void exitGameMode();
GlobalState& getGlobalState();
void initGlobal();
void updateShowEditor(bool showEditor);
void queryUpdateShowEditor(bool showEditor);
bool queryConsoleCanEnable();

bool leftMouseDown();
bool rightMouseDown();
bool middleMouseDown();

#endif 