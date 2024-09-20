#include "./entity.h"

std::unordered_map<objid, ControllableEntity> controllableEntities;

void onAddControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idAdded){
  maybeAddMovementEntity(movementEntities, idAdded);

  auto agent = getSingleAttr(idAdded, "agent");
  if (agent.has_value()){
    addAiAgent(aiData, idAdded, agent.value());
    controllableEntities[idAdded] = ControllableEntity {
      .gunCore = createGunCoreInstance("pistol", 5, gameapi -> listSceneId(idAdded)),
    };
  }
}

void maybeRemoveControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idRemoved){
  maybeRemoveMovementEntity(movementEntities, idRemoved);
  maybeRemoveAiAgent(aiData, idRemoved);
  controllableEntities.erase(idRemoved);
}

