#ifndef MOD_AFTERWORLD_ENTITY
#define MOD_AFTERWORLD_ENTITY

#include "../core/weapons/weaponcore.h"
#include "../core/movement/movement.h"
#include "../core/vehicles/vehicles.h"
#include "../core/scene_core.h"
#include "../core/health.h"
#include "../core/ragdoll.h"
#include "../ai/ai.h"
#include "../curves.h"
#include "../options.h"

struct ControlledPlayer {
	int viewport; // this is used interchangably with the player index (eg player 1, 2, etc)
	glm::vec2 lookVelocity;
	std::optional<objid> entityId;
	std::optional<objid> activePlayerManagedCameraId;
	std::optional<objid> tempCamera;
	bool freeCamera;
	bool disablePlayerControl;  // this maybe should be in global? 

	glm::vec3 shakeOffset = glm::vec3(0.f, 0.f, 0.f);
	glm::vec3 shakeVelocity = glm::vec3(0.f, 0.f, 0.f);
	std::optional<glm::vec3> shakeImpulse;
};

struct ControllableEntity {
	bool isInShootingMode;  // not sure if this should be in here but w/e
	bool zoomIntoArcade;
	bool isAlive;
	std::optional<objid> lookingAtVehicle;
	std::optional<objid> vehicle;
	std::set<objid> disableAnimationIds;
	std::optional<int> zoomAmount;
};

ControlledPlayer& getControlledPlayer(int playerIndex);
ControlledPlayer& getMainControlledPlayer();
int getNumberOfPlayers();
void setMainPlayerControl(int playerIndex);
void addPlayerPort(int playerIndex);
void removePlayerPort(int playerIndex);
bool isControlledPlayer(int playerId);
bool isControlledVehicle(int vehicleId);
int getDefaultPlayerIndex();

void onAddEntity(objid idAdded);
void onRemoveEntity(objid idRemoved);

std::optional<objid> getEntityForPlayerIndex(int playerIndex);
void setEntityForPlayerIndex(std::optional<objid> id, int playerIndex);

void setEntityNextForPlayerIndex(int playerIndex);
void observeEntity(std::optional<objid> id, int playerIndex);
void observeEntityNext(int playerIndex);

std::optional<int> getPlayerIndexForEntity(objid id);
std::vector<ControlledPlayer>& getPlayers();
std::optional<objid> getCameraForThirdPerson(int playerIndex);

void setEntityInShootingMode(objid id, bool shootingMode);
std::optional<bool> isEntityInShootingMode(objid id);

void setEntityIsAlive(objid id, bool alive);
std::optional<bool> activePlayerAlive(int playerIndex);

void setEntityIsFalling(objid id, bool falling);
std::optional<bool> activePlayerFalling(int playerIndex);

void setEntityIsReloading(objid id, bool reloading);
std::optional<bool> activePlayerReloading(int playerIndex);

bool isEntityGunZoomed(objid id);

void setEntityThirdPerson(objid id);
void setEntityFirstPerson(objid id);



bool allPlayersDead();



void onActivePlayerRemoved(objid id);
void setDisablePlayerControl(bool isDisabled, int playerIndex);
bool isPlayerControlDisabled(int playerIndex);

void setPlayerFreeCamera(int playerIndex, bool editorMode);

void setTempCamera(std::optional<objid> camera, int playerIndex);

void killPlayer(int playerIndex);

glm::vec3 getPlayerVelocity(int playerIndex);
std::optional<glm::vec3> getActivePlayerPosition(int playerIndex);

WeaponEntityData getWeaponEntityData(objid id);

DebugConfig debugPrintActivePlayer(int playerIndex);

std::optional<ControllableEntity*> getActiveControllable(int playerIndex);


void disableEntity(objid id);
void reenableEntity(objid id, std::optional<glm::vec3> pos, std::optional<glm::quat> rot);

void deliverCurrentGunAmmo(objid id, int ammoAmount);


void entityEnterVehicle(int playerIndex, objid vehicleId, objid id);
void entityExitVehicle(int playerIndex, objid vehicleId, objid id);

void applyScreenshakeByPlayerIndex(int playerIndex, glm::vec3 impulse);

void zoomIntoArcade(std::optional<objid> id, int playerIndex);
std::optional<bool> isZoomedIntoArcade(int playerIndex);

#endif 