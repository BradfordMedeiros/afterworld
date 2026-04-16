#ifndef MOD_AFTERWORLD_COLLISION
#define MOD_AFTERWORLD_COLLISION

#include "./util.h"
#include "./resources/resources.h"
#include "./core/inventory.h"
#include "./director/director.h"

void handleInteract(objid gameObjId);
void handleCollision(objid obj1, objid obj2, std::string attrForValue, std::string attrForKey, std::string removeKey);

void handleMomentumCollision(objid obj1, objid obj2, glm::vec3 position, glm::quat direction, float force);
void handleInventoryOnCollision(int32_t obj1, int32_t obj2);

void handleTriggerZone(int32_t obj1, int32_t obj2);
void handleOnTriggerEnter(objid obj1, objid obj2);
void handleOnTriggerExit(objid obj1, objid obj2);
void handleOnTriggerRemove(objid triggerZone);

void handleSurfaceCollision(int32_t obj1, int32_t obj2);
void removeSurfaceModifier(int32_t obj1, int32_t obj2);
void removeSurfaceModifier(int32_t id);

void handleLevelEndCollision(int32_t obj1, int32_t obj2);
void handlePowerupCollision(int32_t obj1, int32_t obj2);
void handleGemCollision(int32_t obj1, int32_t obj2);


#endif 