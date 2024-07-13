#ifndef MOD_AFTERWORLD_VECTOR_GFX
#define MOD_AFTERWORLD_VECTOR_GFX

#include "./util.h"

void drawBloom(objid playerId, objid id, float distance, float bloomAmount);
void drawSphereVecGfx(glm::vec3 position, float radius, glm::vec4 tint);
void drawDebugHitmark(HitObject& hitpoint, objid playerId);

#endif