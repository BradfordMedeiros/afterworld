#ifndef MOD_AFTERWORLD_ENTITY
#define MOD_AFTERWORLD_ENTITY

#include "./weapons/weaponcore.h"
#include "./movement/movement.h"
#include "./ai/ai.h"
#include "./health.h"

struct ControlledPlayer {
	glm::vec2 lookVelocity;
	std::optional<objid> playerId;
	std::optional<objid> activePlayerManagedCameraId;
	std::optional<objid> activeMeshManagedId;
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

void setActivePlayerEditorMode(bool editorMode);

void killActivePlayer();

glm::vec3 getPlayerVelocity();
std::optional<glm::vec3> getActivePlayerPosition();

WeaponEntityData getWeaponEntityData(objid id);

DebugConfig debugPrintActivePlayer();


#endif 