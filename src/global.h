#ifndef MOD_AFTERWORLD_GLOBAL
#define MOD_AFTERWORLD_GLOBAL

#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"

struct GlobalState {
  bool paused;
  bool showScreenspaceGrid;

  float xNdc;
  float yNdc;
  std::optional<objid> selectedId;
};

void setPaused(bool paused);
bool isPaused();
GlobalState& getGlobalState();

#endif 