#ifndef MOD_AFTERWORLD_GAMETYPES
#define MOD_AFTERWORLD_GAMETYPES

#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"
#include "./gametypes/targetkill.h"
#include "./gametypes/deathmatch.h"
#include "./gametypes/race.h"

CScriptBinding gametypesBinding(CustomApiBindings& api, const char* name);

#endif 