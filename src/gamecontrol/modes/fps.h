#ifndef MOD_AFTERWORLD_MODE_FPS
#define MOD_AFTERWORLD_MODE_FPS

#include "../gametypes.h"
#include "../../global.h"
#include "../../core/vehicles/vehicles.h"
#include "../../core/scene_core.h"
#include "../../gameworld/progress.h"
#include "../../interaction/cutscene.h"
#include "../../collision.h"
#include "../entity.h"
#include "../../director/director.h"
#include "../../ui/views/playing.h"

void startFpsMode(objid sceneId, std::optional<std::string> player, bool makePlayer);
void stopFpsMode();

#endif 