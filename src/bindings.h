#ifndef MOD_AFTERWORLD_BINDING
#define MOD_AFTERWORLD_BINDING

#include <iostream>
#include <vector>
#include "./movement.h"
#include "./menu.h"
#include "./daynight.h"
#include "./weapon.h"

std::vector<CScriptBinding> getUserBindings(CustomApiBindings& api);

#endif