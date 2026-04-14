#ifndef MOD_AFTERWORLD_MODE_INTRO
#define MOD_AFTERWORLD_MODE_INTRO

#include "../../gametypes/gametypes.h"
#include "../../global.h"
#include "../../core/vehicles/vehicles.h"
#include "../../core/scene_core.h"
#include "../../gameworld/progress.h"
#include "../../orbs.h"
#include "../../interaction/cutscene.h"
#include "../../collision.h"
#include "../entity.h"

void startIntroMode(objid sceneId);
void endIntroMode();

void onModeOrbSelect(std::vector<OrbSelection>& selectedOrbs);
void ballModeNewGame();
void ballModeLevelSelect();

#endif 