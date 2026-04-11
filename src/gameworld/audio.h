#ifndef MOD_AFTERWORLD_AUDIO
#define MOD_AFTERWORLD_AUDIO

#include <iostream>
#include <vector>
#include <ctime>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../resources/resources.h"

extern std::string defaultAudioClipPath;
void ensureAmbientSound(glm::vec3 cameraPos, int changedLevelFrame, bool hasManagedScene);


/////////////
struct CurrentPlayingData {
	objid id;
	objid sceneId;
	std::string clipToPlay;
};

struct AudioZones {
	std::set<objid> audiozoneIds;
	std::optional<CurrentPlayingData> currentPlaying;
};

void addAudioZone(objid id);
void removeAudioZone(objid id);
void onAudioZoneFrame();

#endif 