#ifndef MOD_AFTERWORLD_GAMETYPES
#define MOD_AFTERWORLD_GAMETYPES

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "./targetkill.h"
#include "./deathmatch.h"
#include "./race.h"

CScriptBinding gametypesBinding(CustomApiBindings& api, const char* name);

#endif 