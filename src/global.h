#ifndef MOD_AFTERWORLD_GLOBAL
#define MOD_AFTERWORLD_GLOBAL

#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"

struct GlobalState {
  bool paused;
  bool inGameMode;
  bool showScreenspaceGrid;

  float xNdc;
  float yNdc;
  std::optional<objid> selectedId;

  bool godMode;
};

void setPaused(bool paused);
bool isPaused();
void enterGameMode();
void exitGameMode();
GlobalState& getGlobalState();
void initGlobal();

#endif 