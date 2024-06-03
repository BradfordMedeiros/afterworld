#ifndef MOD_AFTERWORLD_COLLISION
#define MOD_AFTERWORLD_COLLISION

#include "./util.h"

void handleInteract(objid gameObjId);
void handleSwitch(std::string switchValue);
void handleCollision(objid obj1, objid obj2, std::string attrForValue, std::string attrForKey, std::string removeKey);
void handleDamageCollision(objid obj1, objid obj2);
void handleMomentumCollision(objid obj1, objid obj2);

#endif 