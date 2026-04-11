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
#include "./gameworld/audio.h"

struct UiData {
  UiContext uiContext;
  HandlerFns uiCallbacks;
};


struct HealthColorObject {
	glm::vec3 lowColor;
	glm::vec3 highColor;
	std::optional<objid> target;
};

struct ExplosionObj {
	float time;
};

struct Powerup {
	std::string type;
	glm::vec4 tint;
	std::optional<int> respawnRateMs;
	std::optional<float> lastRemoveTime;

	bool disabledVisually;
};


void onTagsMessage(std::string& key, std::any& value);
void onTagsFrame();
void handleOnAddedTags(int32_t idAdded);
void handleOnAddedTagsInitial();
void handleTagsOnObjectRemoved(int32_t idRemoved);

void setMenuBackground(std::string background);


void handleOrbControls(int key, int action);

#endif 