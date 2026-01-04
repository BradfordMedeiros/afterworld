#ifndef MOD_AFTERWORLD_COLLISION
#define MOD_AFTERWORLD_COLLISION

#include "./util.h"
#include "./resources/resources.h"
#include "./inventory.h"
#include "./director/director.h"

void handleInteract(objid gameObjId);
void handleSwitch(std::string switchValue);
void handleCollision(objid obj1, objid obj2, std::string attrForValue, std::string attrForKey, std::string removeKey);
void handleDamageCollision(objid obj1, objid obj2);
void handleMomentumCollision(objid obj1, objid obj2, glm::vec3 position, glm::quat direction, float force);
void handleBouncepadCollision(objid obj1, objid obj2, glm::vec3 normal);
void handleInventoryOnCollision(int32_t obj1, int32_t obj2);
void handleSpawnCollision(int32_t obj1, int32_t obj2);

void handleLevelEndCollision(int32_t obj1, int32_t obj2);
void handleTeleportCollision(int32_t obj1, int32_t obj2);
void handlePowerupCollision(int32_t obj1, int32_t obj2);

void showTriggerVolumes(bool showTriggerVolumes);

#endif 