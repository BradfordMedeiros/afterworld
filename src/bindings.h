#ifndef MOD_AFTERWORLD_BINDING
#define MOD_AFTERWORLD_BINDING

#include <iostream>
#include <vector>
#include "./movement.h"
#include "./menu.h"
#include "./daynight.h"
#include "./weapon.h"
#include "./hud.h"
#include "./tags.h"
#include "./dialog.h"
#include "./debug.h"

std::vector<CScriptBinding> getUserBindings(CustomApiBindings& api);

#endif