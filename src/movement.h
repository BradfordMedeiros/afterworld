#ifndef MOD_AFTERWORLD_MOVEMENT
#define MOD_AFTERWORLD_MOVEMENT

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"
#include "./global.h"
#include "./resources.h"

CScriptBinding movementBinding(CustomApiBindings& api, const char* name);

#endif