#ifndef MOD_AFTERWORLD_COMPONENTS_EDITORVIEW
#define MOD_AFTERWORLD_COMPONENTS_EDITORVIEW

#include "../components/common.h"
#include "../components/worldplay.h"

struct EditorViewOptions {
  WorldPlayInterface* worldPlayInterface;
};
extern Component editorViewComponent;

#endif

