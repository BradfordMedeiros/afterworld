#ifndef MOD_AFTERWORLD_MODE_FPS
#define MOD_AFTERWORLD_MODE_FPS

#include "../gametypes/gametypes.h"
#include "../../global.h"
#include "../../core/vehicles/vehicles.h"
#include "../../core/scene_core.h"
#include "../../gameworld/progress.h"
#include "../../orbs.h"
#include "../../interaction/cutscene.h"
#include "../../collision.h"
#include "../entity.h"

void startFpsMode(objid sceneId, std::optional<std::string> player, bool makePlayer);
void stopFpsMode();

#endif 