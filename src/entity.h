#ifndef MOD_AFTERWORLD_ENTITY
#define MOD_AFTERWORLD_ENTITY

#include "./weapons/weaponcore.h"
#include "./movement/movement.h"
#include "./ai/ai.h"

struct ControlledPlayer {
	std::optional<objid> activePlayerId;
	std::optional<objid> activePlayerManagedCameraId;
	std::optional<objid> tempViewpoint;
	bool editorMode;
};

struct ControllableEntity {
  std::optional<GunCore> gunCore;
};

void onAddControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idAdded);
void maybeRemoveControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idRemoved);

std::optional<objid> getActivePlayerId();
void setActivePlayer(std::optional<objid> id);
void onActivePlayerRemoved(objid id);


void setTempViewpoint(glm::vec3 position, glm::quat rotation);
bool hasTempViewpoint();
void popTempViewpoint();
void setActivePlayerEditorMode(bool editorMode);
DebugConfig debugPrintActivePlayer();


#endif 