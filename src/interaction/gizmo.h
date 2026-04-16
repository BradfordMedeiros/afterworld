#ifndef MOD_AFTERWORLD_GIZMO
#define MOD_AFTERWORLD_GIZMO

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "../global.h"
#include "../debug.h"

struct GlassTexture {
	objid id;
	std::string name;
};

void createGlassTexture(objid id);
void removeGlassTexture(objid id);
bool maybeAddGlassBulletHole(objid id, objid playerId);

///////////

struct Laser { };
void addLaser(objid id, float length);
void removeLaser(objid id);
void onLaserFrame();


//////////
struct GravityWell {
	objid id;
	std::optional<std::string> name;
	std::optional<std::string> target;

	bool autolaunch;

	std::optional<objid> managedItem;
	std::optional<glm::vec3> launcher;
};

std::optional<GravityWell*> gravityWellByManaged(objid managed);
bool addToGravityWell(objid gravityWellId, objid managed);
void removeFromGravityWell(objid managed);
glm::vec3 getTargetWellPosition(GravityWell& gravityWell);
void onFrameGravityWells();

// Go to the closest gravity well in the direction specified by the direction
std::optional<glm::vec3> goToNextGravityWell(objid managed, glm::vec3 moveDirection);
bool shouldAutolaunchGravityWell(objid managed);


//////////////////
struct TriggerColor {
	std::string trigger;
	std::optional<glm::vec4> activeColor;
	std::optional<glm::vec4> unactiveColor;
};

void createExplosion(glm::vec3 position, float outerRadius, float damage);
void applyImpulseAffectMovement(objid id, glm::vec3 force);
std::optional<glm::vec3> getImpulseThisFrame(objid id);


struct LinkGunObj {};
void addLinkGunObj(objid id);
void removeLinkGunObj(objid id);
void onLinkGunObjFrame();

struct TeleportExit {
	std::optional<std::string> exit;
};
struct TeleportInfo {
	objid id;
	glm::vec3 position;
};

void handleTeleport(objid idToTeleport, objid teleporterId);
void doTeleport(int32_t idToTeleport, std::string destination);
std::optional<TeleportInfo> getTeleportPosition();
void handleCollisionTeleport(int32_t obj1, int32_t obj2);

struct SpinObject {
	float timeAdded;
};
/////// Minor effect coloring, rotation, etc
void startRotate(objid id);
void stopRotate(objid id);
void onRotateFrame(bool inGameMode);

struct EmissionObject {
	glm::vec3 lowColor;
	glm::vec3 highColor;
	float period;
};

void addEmissionObj(objid id, glm::vec3 lowColor, glm::vec3 highColor, float period);
void removeEmissionObj(objid id);
void onEmissionFrame();


void handleScroll(std::set<objid>& textureScrollObjIds);

void handleCollisionBouncepad(objid obj1, objid obj2, glm::vec3 normal);

#endif 