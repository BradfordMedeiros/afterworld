#ifndef MOD_AFTERWORLD_SURFACE
#define MOD_AFTERWORLD_SURFACE

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"

struct ExtraSurfaceVelocity {
  objid surfaceId;
  glm::vec3 velocity;
};

void handleSurfaceCollision(int32_t obj1, int32_t obj2);
void removeSurfaceModifier(int32_t obj1, int32_t obj2);
void removeSurfaceModifier(int32_t id);

#endif