#ifndef MOD_AFTERWORLD_DEBUG
#define MOD_AFTERWORLD_DEBUG

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./global.h"
#include "./controls.h"
#include "./spawn.h"
#include "./cutscene.h"
#include "./tags.h"
#include "./options.h"
#include "./entity.h"
#include "./gametypes/gametypes.h"
#include "./progression/progress.h"

void debugOnFrame();
void debugOnKey(int key, int scancode, int action, int mods);

// nice for debugging, but don't use permanently 
// since hard to eg save state etc 
void simpleOnFrame(std::function<void()> fn, float duration);

#endif 