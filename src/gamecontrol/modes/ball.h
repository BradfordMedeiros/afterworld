#ifndef MOD_AFTERWORLD_MODE_BALL
#define MOD_AFTERWORLD_MODE_BALL

#include "../gametypes/gametypes.h"
#include "../../global.h"
#include "../../core/vehicles/vehicles.h"
#include "../../core/scene_core.h"
#include "../../gameworld/progress.h"
#include "../../orbs.h"
#include "../../interaction/cutscene.h"
#include "../../interaction/gizmo.h"
#include "../../collision.h"
#include "../entity.h"

void startBallMode(objid sceneId);
void endBallMode();

void handleGravityHoleCollision(objid obj1, objid obj2);
void handleLevelEndCollision(int32_t obj1, int32_t obj2);
void handlePowerupCollision(int32_t obj1, int32_t obj2);

#endif 