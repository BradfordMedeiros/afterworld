#ifndef MOD_AFTERWORLD_TAGS
#define MOD_AFTERWORLD_TAGS

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"
#include "./state-controller.h"
#include "./spawn.h"
#include "./global.h"
#include "./activeplayer.h"

CScriptBinding tagsBinding(CustomApiBindings& api, const char* name);

#endif 