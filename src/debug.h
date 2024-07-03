#ifndef MOD_AFTERWORLD_DEBUG
#define MOD_AFTERWORLD_DEBUG

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./global.h"
#include "./controls.h"
#include "./spawn.h"
#include "./cutscene.h"

void debugOnFrame();
void debugOnKey(int key, int scancode, int action, int mods);


#endif 