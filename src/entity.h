#ifndef MOD_AFTERWORLD_ENTITY
#define MOD_AFTERWORLD_ENTITY

#include "./weapons/weaponcore.h"
#include "./movement/movement.h"
#include "./ai/ai.h"
#include "./health.h"
#include "./curves.h"

struct ControlledPlayer {
	glm::vec2 lookVelocity;
	std::optional<objid> playerId;
	std::optional<objid> activePlayerManagedCameraId;
	std::optional<objid> tempCamera;
	bool editorMode;
	bool disablePlayerControl;  // this maybe should be in global? 
	std::set<objid> disableAnimationIds;
};

struct ControllableEntity {
	bool isInShootingMode;  // not sure if this should be in here but w/e
};

void onAddControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idAdded);
void maybeRemoveControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idRemoved);
bool controllableEntityExists(objid id);

std::optional<objid> findBodyPart(objid entityId, const char* part);  // should move into a util

std::optional<objid> getActivePlayerId();
std::optional<objid> getCameraForThirdPerson();
std::optional<bool> activePlayerInThirdPerson();

std::optional<bool> isInShootingMode(objid id);
void setInShootingMode(objid id, bool shootingMode);

void maybeReEnableMesh(objid id);
void maybeDisableMesh(objid id);

void setActivePlayer(Movement& movement, Weapons& weapons, AiData& aiData, std::optional<objid> id);
void setActivePlayerNext(Movement& movement, Weapons& weapons, AiData& aiData);
bool onActivePlayerRemoved(objid id);
void setDisablePlayerControl(bool isDisabled);
bool isPlayerControlDisabled();

void setActivePlayerEditorMode(bool editorMode);
bool isInGameMode();

void setTempCamera(std::optional<objid> camera);
std::optional<objid> getTempCamera();

void killActivePlayer();

glm::vec3 getPlayerVelocity();
std::optional<glm::vec3> getActivePlayerPosition();
std::optional<glm::quat> getActivePlayerRotation();

WeaponEntityData getWeaponEntityData(objid id);

DebugConfig debugPrintActivePlayer();

#endif 