#ifndef MOD_AFTERWORLD_ENTITY
#define MOD_AFTERWORLD_ENTITY

#include "./weapons/weaponcore.h"
#include "./movement/movement.h"
#include "./ai/ai.h"

struct ControllableEntity {
  std::optional<GunCore> gunCore;
};

void onAddControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idAdded);
void maybeRemoveControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idRemoved);

#endif 