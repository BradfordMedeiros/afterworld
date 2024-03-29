#ifndef MOD_AFTERWORLD_AI
#define MOD_AFTERWORLD_AI

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "./worldinfo.h"
#include "./agents/basic_agent.h"
#include "./agents/turret.h"
#include "../global.h"

CScriptBinding aiBinding(CustomApiBindings& api, const char* name);

#endif