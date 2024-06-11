#ifndef MOD_AFTERWORLD_COLLISION
#define MOD_AFTERWORLD_COLLISION

#include "./util.h"
#include "./resources.h"

void handleInteract(objid gameObjId);
void handleSwitch(std::string switchValue);
void handleCollision(objid obj1, objid obj2, std::string attrForValue, std::string attrForKey, std::string removeKey);
void handleDamageCollision(objid obj1, objid obj2);
void handleMomentumCollision(objid obj1, objid obj2, glm::vec3 position, glm::quat direction, float force);
void handleBouncepadCollision(objid obj1, objid obj2, glm::vec3 normal);

#endif 