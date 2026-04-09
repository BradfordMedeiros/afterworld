#ifndef MOD_AFTERWORLD_VEHICLES_COMMON
#define MOD_AFTERWORLD_VEHICLES_COMMON

#include "../../../../../ModEngine/src/cscript/cscript_binding.h"
#include "../../util.h"
#include "../../controls.h"
#include "../../core/movement/movementcore.h"

struct VehicleState {
  std::optional<objid> occupied;

  glm::vec3 controls;
  ThirdPersonCameraInfo managedCamera;

  std::optional<objid> sound;
};


#endif

