#ifndef MOD_AFTERWORLD_DEBUG
#define MOD_AFTERWORLD_DEBUG

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./global.h"
#include "./controls.h"

CScriptBinding debugBinding(CustomApiBindings& api, const char* name);

#endif 