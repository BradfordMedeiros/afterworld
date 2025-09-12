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
};

struct ControllableEntity {
	bool isInShootingMode;  // not sure if this should be in here but w/e
	bool isAlive;
	std::optional<objid> lookingAtVehicle;
	std::optional<objid> vehicle;
	std::set<objid> disableAnimationIds;
};

void onAddControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idAdded);
void maybeRemoveControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idRemoved);
bool controllableEntityExists(objid id);

std::optional<objid> findBodyPart(objid entityId, const char* part);  // should move into a util
std::vector<objid> findBodyPartAndChilren(objid entityId, const char* part);

std::optional<objid> getActivePlayerId();
std::optional<objid> getCameraForThirdPerson();
std::optional<bool> activePlayerInThirdPerson();

std::optional<bool> isInShootingMode(objid id);
void setInShootingMode(objid id, bool shootingMode);
void setIsAlive(objid id, bool alive);
void setIsFalling(objid id, bool falling);
void setIsReloading(objid id, bool reloading);

void maybeReEnableMesh(objid id);
void maybeDisableMesh(objid id);

void setActivePlayer(Movement& movement, Weapons& weapons, AiData& aiData, std::optional<objid> id);
void setActivePlayerNext(Movement& movement, Weapons& weapons, AiData& aiData);
bool onActivePlayerRemoved(objid id);
void setDisablePlayerControl(bool isDisabled);
bool isPlayerControlDisabled();
std::optional<bool> activePlayerAlive();
std::optional<bool> activePlayerFalling();
std::optional<bool> activePlayerReloading();

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

std::optional<ControllableEntity*> getActiveControllable();

void setEntityThirdPerson(objid id);
void setEntityFirstPerson(objid id);
void disableEntity(objid id);
void reenableEntity(objid id, std::optional<glm::vec3> pos, std::optional<glm::quat> rot);

void createHitbox(objid id);
void enterRagdoll(objid id);

#endif 