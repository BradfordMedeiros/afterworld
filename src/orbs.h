#ifndef MOD_AFTERWORLD_ORBS
#define MOD_AFTERWORLD_ORBS

#include <string>
#include <set>
#include <vector>
#include <unordered_map>
#include <optional>
#include "./util.h"
#include "./vector_gfx.h"
#include "./controls.h"

struct OrbView {
	objid orbId;
	int actualIndex;
	int targetIndex;
	std::optional<float> startTime;
};
struct Orb {
	int index;
	glm::vec3 position;
	glm::vec4 tint;
	std::string text;

	std::string level;
};
struct OrbConnection {
	int indexFrom;
	int indexTo;
};
struct OrbUi {
	objid id;
	std::vector<Orb> orbs;
	std::vector<OrbConnection> connections;
};
struct OrbData {
	std::unordered_map<objid, OrbUi> orbUis;
	std::unordered_map<objid, OrbView> orbViewsCameraToOrb;
};

std::optional<Orb*> getOrb(std::vector<Orb>& orbs, int index);
glm::vec3 getOrbPosition(OrbUi& orbUi, int index);
glm::quat getOrbRotation(OrbUi& orbUi, int index);

void drawOrbs(OrbUi& orbUi, int ownerId);

void handleOrbViews(OrbData& orbData);

struct OrbSelection {
	std::optional<Orb*> selectedOrb;
};	
OrbSelection handleOrbControls(OrbData& orbData, int key, int action);

std::string print(Orb& orb);

#endif 