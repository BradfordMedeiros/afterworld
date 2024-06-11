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
#include "./in-game-ui.h"
#include "./conditional_spawn.h"

CScriptBinding tagsBinding(CustomApiBindings& api, const char* name);

std::optional<glm::vec3> getTeleportPosition();

#endif 