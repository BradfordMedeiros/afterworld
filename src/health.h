#ifndef MOD_AFTERWORLD_HEALTH
#define MOD_AFTERWORLD_HEALTH

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"
#include "./global.h"
#include "./activeplayer.h"

void addEntityIdHitpoints(objid id);
void removeEntityIdHitpoints(objid id);
void doDamageMessage(objid targetId, float damageAmount);

#endif