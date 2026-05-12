#ifndef MOD_AFTERWORLD_ORBS
#define MOD_AFTERWORLD_ORBS

#include <string>
#include <set>
#include <vector>
#include <unordered_map>
#include <optional>
#include "../util.h"
#include "../vector_gfx.h"
#include "../controls.h"

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
void drawOrbs(OrbData& orbData, OrbUi& orbUi, int ownerId);
void handleOrbViews(OrbData& orbData);


void setCameraToOrbView(objid cameraId, std::string orbUiName, std::optional<int> targetIndex, std::optional<float> time);
void removeCameraFromOrbView(objid cameraId);
void handleOrbUiRemoved(objid id);

std::optional<int> getMaxCompleteOrbIndex(OrbUi& orbUi);
std::optional<OrbUi*> orbUiByName(std::string orbUiName);
std::optional<int>  getActiveOrbViewIndex(objid cameraId);

void setOrbSelectIndex(OrbView& orbView, int targetIndex);

int getPrevOrbIndex(OrbUi& orbUi, int targetIndex);
int getNextOrbIndex(OrbUi& orbUi, int targetIndex);

std::optional<Orb*> selectedOrbForCamera(objid cameraId);

std::string print(Orb& orb);


//// multi view
struct MultiOrbView {
	std::optional<objid> orbCameraId;
	bool onOverworld;
};

void setToMultiOrbView(objid cameraId, std::optional<std::string> world);
void removeCameraFromMultiOrbView(objid cameraId);
void prevOrb(MultiOrbView& multiOrbView);
void nextOrb(MultiOrbView& multiOrbView);

bool isOverworld(MultiOrbView& multiOrbView);

void goToOverWorld(MultiOrbView& multiOrbView, bool onOverworld);

std::optional<std::string> getSelectedLevel(MultiOrbView& multiOrbView);
std::optional<MultiOrbView*> multiorbViewByCamera(objid cameraId);

struct WorldOrbInfos {
	std::string level;
	bool isComplete;
	bool selected;
};
std::vector<WorldOrbInfos> getOrbUiData(MultiOrbView& multiOrbView);


#endif 