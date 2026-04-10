#ifndef MOD_AFTERWORLD_TAGS
#define MOD_AFTERWORLD_TAGS

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"
#include "./animation/animation.h"
#include "./director/director.h"
#include "./global.h"
#include "./interaction/in-game-ui.h"
#include "./interaction/gizmo.h"
#include "./core/health.h"
#include "./core/vehicles/vehicles.h"
#include "./debug.h"
#include "./vector_gfx.h"
#include "./arcade/arcade.h"
#include "./entity.h"
#include "./orbs.h"
#include "./config.h"

struct CurrentPlayingData {
	objid id;
	objid sceneId;
	std::string clipToPlay;
};
struct AudioZones {
	std::set<objid> audiozoneIds;
	std::optional<CurrentPlayingData> currentPlaying;
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
struct HealthColorObject {
	glm::vec3 lowColor;
	glm::vec3 highColor;
	std::optional<objid> target;
};

struct ExplosionObj {
	float time;
};

struct LinkGunObj {

};

struct TeleportExit {
	std::optional<std::string> exit;
};

struct Powerup {
	std::string type;
	glm::vec4 tint;
	std::optional<int> respawnRateMs;
	std::optional<float> lastRemoveTime;

	bool disabledVisually;
};


struct SpinObject {
	float timeAdded;
};

struct Tags {
	std::set<objid> textureScrollObjIds;
	AudioZones audiozones;
	InGameUi inGameUi;
	std::unordered_map<objid, SpinObject> idToRotateTimeAdded;
	std::unordered_map<objid, EmissionObject> emissionObjects;
	std::unordered_map<objid, HealthColorObject> healthColorObjects;
	std::unordered_map<objid, TeleportExit> teleportObjs;
	std::unordered_map<objid, ExplosionObj> explosionObjects;
	std::unordered_map<objid, LinkGunObj> linkGunObj;
	std::unordered_map<objid, Powerup> powerups;

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

void handleOrbControls(int key, int action);

#endif 