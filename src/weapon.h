#ifndef MOD_AFTERWORLD_WEAPON
#define MOD_AFTERWORLD_WEAPON

#include <iostream>
#include <vector>
#include <regex>
#include "../../ModEngine/src/scene/serialization.h"
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"

CScriptBinding weaponBinding(CustomApiBindings& api, const char* name);

#endif