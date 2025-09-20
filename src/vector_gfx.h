#ifndef MOD_AFTERWORLD_VECTOR_GFX
#define MOD_AFTERWORLD_VECTOR_GFX

#include "./util.h"

void drawBloom(objid playerId, objid id, float distance, float bloomAmount);
void drawSphereVecGfx(glm::vec3 position, float radius, glm::vec4 tint);

void setDrawDebugVector(bool shouldDrawDebugVector);
void drawDebugHitmark(HitObject& hitpoint, objid playerId);
void drawDebugRaycast(glm::vec3 fromPosition, glm::vec3 toPos, objid playerId);

#endif