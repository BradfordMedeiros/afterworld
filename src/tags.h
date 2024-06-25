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
#include "./health.h"

CScriptBinding tagsBinding(CustomApiBindings& api, const char* name);

struct TeleportInfo {
	objid id;
	glm::vec3 position;
};

std::optional<TeleportInfo> getTeleportPosition();

void doAnimationTrigger(objid entityId, const char* transition);

#endif 