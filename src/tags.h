#ifndef MOD_AFTERWORLD_TAGS
#define MOD_AFTERWORLD_TAGS

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"
#include "./animation/animation.h"
#include "./spawn.h"
#include "./global.h"
#include "./in-game-ui.h"
#include "./health.h"
#include "./debug.h"
#include "./vector_gfx.h"
#include "./switch.h"
#include "./arcade/arcade.h"

struct CurrentPlayingData {
	objid id;
	objid sceneId;
	std::string clipToPlay;
};
struct AudioZones {
	std::set<objid> audiozoneIds;
	std::optional<CurrentPlayingData> currentPlaying;
};

struct ManagedRecording{
	std::string signal;
};

struct UiData {
  UiContext uiContext;
  HandlerFns uiCallbacks;
};

struct EmissionObject {
	glm::vec3 lowColor;
	glm::vec3 highColor;
	float period;
};


struct Tags {
	std::set<objid> textureScrollObjIds;
	AudioZones audiozones;
	InGameUi inGameUi;
	std::unordered_map<objid, float> idToRotateTimeAdded;
	std::unordered_map<objid, EmissionObject> emissionObjects;
	std::set<objid> teleportObjs;
	std::unordered_map<objid, ManagedRecording> recordings;
	Switches switches;

	UiData* uiData;

	StateController animationController;
};

Tags createTags(UiData* uiData);
void onTagsMessage(Tags& tags, std::string& key, std::any& value);
void onTagsFrame(Tags& tags);
void handleOnAddedTags(Tags& tags, int32_t idAdded);
void handleOnAddedTagsInitial(Tags& tags);
void handleTagsOnObjectRemoved(Tags& tags, int32_t idRemoved);

void setMenuBackground(std::string background);

struct TeleportInfo {
	objid id;
	glm::vec3 position;
};

std::optional<TeleportInfo> getTeleportPosition(Tags& tags);

void playRecordingBySignal(std::string signal, std::string rec, bool reverse);

#endif 