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
#include "../ui/views/settings.h"

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


////////////////////// Core Add / Rm  //////////////////////
void onAddEntity(objid idAdded);
void onRemoveEntity(objid idRemoved);

////////////////////// Player port / viewport functions 
ControlledPlayer& getControlledPlayer(int playerIndex);
ControlledPlayer& getMainControlledPlayer();
int getNumberOfPlayers();
void setMainPlayerControl(int playerIndex);
void addPlayerPort(int playerIndex);
void removePlayerPort(int playerIndex);
bool isControlledPlayer(int playerId);
int getDefaultPlayerIndex();
void setPlayerControlDisabled(bool isDisabled, int playerIndex);
bool isPlayerControlDisabled(int playerIndex);

////////////////////// Lookup Utilities //////////////////////
std::optional<int> getPlayerIndexForEntity(objid id);
std::optional<objid> getEntityForPlayerIndex(int playerIndex);
std::optional<ControllableEntity*> getActiveControllable(int playerIndex);
std::vector<ControlledPlayer>& getPlayers();

////////////////////// Core Entity To Viewport Binding //////////////////////
void setEntityForPlayerIndex(std::optional<objid> id, int playerIndex);
void setEntityNextForPlayerIndex(int playerIndex);
void observeEntity(std::optional<objid> id, int playerIndex);
void observeEntityNext(int playerIndex);

std::optional<objid> getCameraForThirdPerson(int playerIndex);

////////////////////// Basic Entity Manipulation //////////////////////
void setEntityInShootingMode(objid id, bool shootingMode);
std::optional<bool> isEntityInShootingMode(objid id);

void setEntityIsAlive(objid id, bool alive);
std::optional<bool> isEntityAlive(objid id);

void setEntityIsFalling(objid id, bool falling);
std::optional<bool> isEntityFalling(objid id);

void setEntityIsReloading(objid id, bool reloading);
std::optional<bool> isEntityReloading(objid id);

void setEntityZoom(objid id, float multiplier);  // TODO restructure to entity id
bool isEntityGunZoomed(objid id);
std::optional<int> getEntityZoomAmount(objid id);

void deliverEntityAmmo(objid id, int ammoAmount);
glm::vec3 getEntityVelocity(objid id);

void killEntity(objid id);

void setEntityThirdPerson(objid id);
void setEntityFirstPerson(objid id);

void setEntityInVehicle(objid id, std::optional<objid> vehicleId);
void entityActionVehicle(objid id);

bool setEntityLookAt(objid id, std::optional<objid> lookAt);

void setEntityFocusArcade(std::optional<objid> arcadeId, objid id);
std::optional<bool> isEntityFocusArcade(objid id);

////////////////////// Convenience Utilities for player indexs //////////////////////
bool allPlayersDead();
std::optional<glm::vec3> getEntityPositionByPlayerIndex(int playerIndex);

////////////////////// Camera Manipulation //////////////////////
void setPlayerFreeCamera(int playerIndex, bool editorMode);
void setTempCamera(std::optional<objid> camera, int playerIndex);
void applyScreenshakeByPlayerIndex(int playerIndex, glm::vec3 impulse);

////////////////////// Glue //////////////////////
WeaponEntityData getWeaponEntityData(objid id);


////////////////////// Misc //////////////////////
DebugConfig debugPrintActivePlayer(int playerIndex);


#endif 