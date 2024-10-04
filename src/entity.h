#ifndef MOD_AFTERWORLD_ENTITY
#define MOD_AFTERWORLD_ENTITY

#include "./weapons/weaponcore.h"
#include "./movement/movement.h"
#include "./ai/ai.h"
#include "./health.h"

struct ControlledPlayer {
  glm::vec3 playerVelocity;
	glm::vec2 lookVelocity;
	std::optional<objid> playerId;
	std::optional<objid> activePlayerManagedCameraId;
	std::optional<objid> tempViewpoint;
	bool editorMode;
};

struct ControllableEntity {
};

void onAddControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idAdded);
void maybeRemoveControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idRemoved);

std::optional<objid> getActivePlayerId();
std::optional<objid> getCameraForThirdPerson();
void setActivePlayer(Movement& movement, Weapons& weapons, AiData& aiData, std::optional<objid> id);
void setActivePlayerNext(Movement& movement, Weapons& weapons, AiData& aiData);
bool onActivePlayerRemoved(objid id);

void setTempViewpoint(glm::vec3 position, glm::quat rotation);
bool hasTempViewpoint();
void popTempViewpoint();
void setActivePlayerEditorMode(bool editorMode);

void killActivePlayer();

glm::vec3 getPlayerVelocity();

WeaponEntityData getWeaponEntityData(objid id);

DebugConfig debugPrintActivePlayer();


#endif 