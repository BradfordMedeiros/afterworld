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
  std::optional<objid> selectedId;

  bool godMode;


  float xsensitivity;
  float ysensitivity;
  bool invertY;
};

void setPaused(bool paused);
bool isPaused();
void enterGameMode();
void exitGameMode();
GlobalState& getGlobalState();
void initGlobal();
void updateShowEditor(bool showEditor);

#endif 