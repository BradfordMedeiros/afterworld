#ifndef MOD_AFTERWORLD_ENTITY
#define MOD_AFTERWORLD_ENTITY

#include "./weapons/weaponcore.h"
#include "./movement/movement.h"
#include "./ai/ai.h"
#include "./health.h"
#include "./curves.h"
#include "./resources/layer.h"

struct ControlledPlayer {
	int viewport;
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

ControlledPlayer& getControlledPlayer(int playerIndex);
void setNumberPlayers(int numPlayers);
int getNumberOfPlayers();
void addPlayerPort(int playerIndex);

void onAddControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idAdded);
void maybeRemoveControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idRemoved);
bool controllableEntityExists(objid id);

std::optional<objid> findBodyPart(objid entityId, const char* part);  // should move into a util
std::vector<objid> findBodyPartAndChilren(objid entityId, const char* part);

int getDefaultPlayerIndex();
std::optional<objid> getActivePlayerId(int playerIndex);
std::optional<objid> getCameraForThirdPerson(int playerIndex);
std::optional<bool> activePlayerInThirdPerson(int playerIndex);

std::optional<bool> isInShootingMode(objid id);
void setInShootingMode(objid id, bool shootingMode);
void setIsAlive(objid id, bool alive);
void setIsFalling(objid id, bool falling);
void setIsReloading(objid id, bool reloading);

void maybeReEnableMesh(objid id);
void maybeDisableMesh(objid id);

void setActivePlayer(Movement& movement, Weapons& weapons, AiData& aiData, std::optional<objid> id, int player);
void setActivePlayerNext(Movement& movement, Weapons& weapons, AiData& aiData, int playerIndex);
void observePlayer(Movement& movement, Weapons& weapons, AiData& aiData, std::optional<objid> id, int playerIndex);
void observePlayerNext(Movement& movement, Weapons& weapons, AiData& aiData, int playerIndex);

bool onActivePlayerRemoved(objid id);
void setDisablePlayerControl(bool isDisabled, int playerIndex);
bool isPlayerControlDisabled(int playerIndex);
std::optional<bool> activePlayerAlive(int playerIndex);
std::optional<bool> activePlayerFalling(int playerIndex);
std::optional<bool> activePlayerReloading(int playerIndex);

void setActivePlayerEditorMode(bool editorMode, int playerIndex);
bool isInGameMode();

void setTempCamera(std::optional<objid> camera, int playerIndex);
std::optional<objid> getTempCamera(int playerIndex);

void killActivePlayer(int playerIndex);

glm::vec3 getPlayerVelocity(int playerIndex);
std::optional<glm::vec3> getActivePlayerPosition(int playerIndex);
std::optional<glm::quat> getActivePlayerRotation(int playerIndex);

WeaponEntityData getWeaponEntityData(objid id);

DebugConfig debugPrintActivePlayer(int playerIndex);

std::optional<ControllableEntity*> getActiveControllable(int playerIndex);

void setEntityThirdPerson(objid id);
void setEntityFirstPerson(objid id);
void disableEntity(objid id);
void reenableEntity(objid id, std::optional<glm::vec3> pos, std::optional<glm::quat> rot);

void createHitbox(objid id);
void enterRagdoll(objid id);

struct RigHit {
	bool isHeadShot;
	objid mainId;
};

std::string print(RigHit& righit);
std::optional<RigHit> handleRigHit(objid id);


#endif 