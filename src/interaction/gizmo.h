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

#endif 