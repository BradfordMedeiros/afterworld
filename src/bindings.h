#ifndef MOD_AFTERWORLD_BINDING
#define MOD_AFTERWORLD_BINDING

#include <iostream>
#include <vector>
#include <stack>
#include "./ai.h"
#include "./movement.h"
#include "./menu.h"
#include "./daynight.h"
#include "./weapon.h"
#include "./vehicle.h"
#include "./inventory.h"
#include "./hud.h"
#include "./tags.h"
#include "./dialog.h"
#include "./debug.h"
#include "./weather.h"
#include "./in-game-ui.h"
#include "./sound.h"
#include "./water.h"
#include "./gametypes.h"
#include "./global.h"

std::vector<CScriptBinding> getUserBindings(CustomApiBindings& api);

#endif