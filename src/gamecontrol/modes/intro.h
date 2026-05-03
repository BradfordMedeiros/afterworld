#ifndef MOD_AFTERWORLD_MODE_INTRO
#define MOD_AFTERWORLD_MODE_INTRO

#include "../gametypes.h"
#include "../../global.h"
#include "../../core/vehicles/vehicles.h"
#include "../../core/scene_core.h"
#include "../../gameworld/progress.h"
#include "../../interaction/orbs.h"
#include "../../interaction/cutscene.h"
#include "../../collision.h"
#include "../entity.h"
#include "../../ui/components/game/fade.h"
#include "../../ui/views/playing.h"

void startIntroMode(objid sceneId);
void endIntroMode();

struct LevelOrbNavInfo {
	std::string orbUi;
	std::optional<int> orbIndex;
	std::optional<int> maxCompletedIndex;
};
struct MultiOrbView {
	std::vector<std::string> orbLayers = { "testorb", "testorb3", "metaworld" };
	int activeLayer = 0;
	std::string activeWorldName;
	std::optional<objid> orbCameraId;
};

#endif 