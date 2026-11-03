#ifndef MOD_AFTERWORLD_MODE_BALL
#define MOD_AFTERWORLD_MODE_BALL

#include "../gametypes.h"
#include "../../global.h"
#include "../../core/vehicles/vehicles.h"
#include "../../core/scene_core.h"
#include "../../gameworld/progress.h"
#include "../../gameworld/triggerzone.h"
#include "../../gameworld/surface.h"
#include "../../interaction/cutscene.h"
#include "../../interaction/gizmo.h"
#include "../../interaction/rhythm.h"
#include "../../collision.h"
#include "../entity.h"

void startBallMode(objid sceneId);
void endBallMode();

void handleBallModeCollision(objid obj1, objid obj2);
void ballModeForceAcquireSoul();

void startSoulRun(std::string song);
void stopSoulRun();

void handleBallModePauseToggle(bool paused);

#endif 