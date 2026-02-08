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

	bool attachedToOrb;
	std::optional<glm::vec3> initialPosition;
	std::optional<glm::quat> initialRotation;
	std::optional<float> moveToOrbStartTime;
	std::optional<float> duration;
};

struct OrbProgress {
	bool complete;
};
struct Orb {
	int index;
	glm::vec3 position;
	glm::quat rotation;
	glm::vec4 tint;
	std::string text;
	std::optional<std::string> mesh;
	std::string level;
	std::optional<std::string> image;
	std::function<OrbProgress()> getOrbProgress;
};
struct OrbConnection {
	int indexFrom;
	int indexTo;
};
struct OrbUi {
	objid ownerId;
	std::string name;
	std::vector<Orb> orbs;
	std::vector<OrbConnection> connections;
};
struct OrbData {
	std::unordered_map<objid, OrbUi> orbUis;
	std::unordered_map<objid, OrbView> orbViewsCameraToOrb;
	std::unordered_map<objid, std::unordered_map<int, objid>> orbIdToIndexToMeshId;

};

std::optional<Orb*> getOrb(std::vector<Orb>& orbs, int index);
glm::vec3 getOrbPosition(OrbUi& orbUi, int index);
glm::quat getOrbRotation(OrbUi& orbUi, int index);

void drawOrbs(OrbData& orbData, OrbUi& orbUi, int ownerId);
void handleOrbViews(OrbData& orbData);

struct OrbSelection {
	objid cameraId;
	std::optional<Orb*> currentOrb;
	OrbView* orbView;
	OrbUi* orbUi;
	bool moveLeft = false;
	bool moveRight = false;

	bool moveLeftKey = false;
	bool moveRightKey = false;
	bool selectKey = false;
};	
std::vector<OrbSelection> handleOrbControls(OrbData& orbData, int key, int action);

void setCameraToOrbView(objid cameraId, std::string orbUiName, std::optional<int> targetIndex, std::optional<float> time);
void removeCameraFromOrbView(objid cameraId);

std::optional<int> getMaxCompleteOrbIndex(OrbUi& orbUi);
std::optional<OrbUi*> orbUiByName(std::string orbUiName);
std::optional<int>  getActiveOrbViewIndex(objid cameraId);

void setOrbSelectIndex(OrbView& orbView, int targetIndex);

int getPrevOrbIndex(OrbUi& orbUi, int targetIndex);
int getNextOrbIndex(OrbUi& orbUi, int targetIndex);
int getMinOrbIndex(OrbUi& orbUi);

int numberOfOrbs(OrbView& orbView);
std::optional<OrbView*> orbViewForCamera(objid cameraId);

std::string print(Orb& orb);


#endif 