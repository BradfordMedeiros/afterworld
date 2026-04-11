#ifndef MOD_AFTERWORLD_SCENE_ANIMATION
#define MOD_AFTERWORLD_SCENE_ANIMATION

#include <iostream>
#include <vector>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "./state-controller.h"

void doAnimationTrigger(objid entityId, const char* transition);
void doStateControllerAnimations(bool validateAnimationControllerAnimations, bool disableAnimation);

#endif 