#ifndef MOD_AFTERWORLD_TAGS
#define MOD_AFTERWORLD_TAGS

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"
#include "./state-controller.h"
#include "./spawn.h"
#include "./global.h"
#include "./activeplayer.h"
#include "./in-game-ui.h"
#include "./conditional_spawn.h"
#include "./health.h"
#include "./debug.h"
#include "./vector_gfx.h"

struct CurrentPlayingData {
	objid id;
	objid sceneId;
	std::string clipToPlay;
};
struct AudioZones {
	std::set<objid> audiozoneIds;
	std::optional<CurrentPlayingData> currentPlaying;
};

enum OpenBehavior {
		OPEN_BEHAVIOR_DELETE, OPEN_BEHAVIOR_UP, OPEN_BEHAVIOR_TOGGLE
};
struct OpenableType {
	std::string signal;
	std::string closeSignal;
	OpenBehavior behavior;
	bool stateUp;
};

struct ManagedRecording{
	std::string signal;
};

struct UiData {
  UiContext uiContext;
  HandlerFns uiCallbacks;
};

struct Tags {
	std::set<objid> textureScrollObjIds;
	AudioZones audiozones;
	InGameUi inGameUi;
	std::unordered_map<objid, OpenableType> openable;
	std::unordered_map<objid, float> idToRotateTimeAdded;
	std::set<objid> teleportObjs;

	std::unordered_map<objid, ManagedRecording> recordings;

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

#endif 