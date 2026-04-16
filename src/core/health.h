#ifndef MOD_AFTERWORLD_HEALTH
#define MOD_AFTERWORLD_HEALTH

#include <iostream>
#include <vector>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "../global.h"
#include "../gamecontrol/entity.h"
#include "../interaction/waypoint.h"

struct HitPoints {
	float current;
	float total;
};

void addEntityIdHitpoints(objid id);
void removeEntityIdHitpoints(objid id);
void doDamageMessage(objid targetId, float damageAmount);
std::optional<HitPoints> getHealth(objid id);
DebugConfig debugPrintHealth();

void handleCollisionDamage(objid obj1, objid obj2);
void handleMomentumCollision(objid obj1, objid obj2, glm::vec3 position, glm::quat direction, float force);

#endif