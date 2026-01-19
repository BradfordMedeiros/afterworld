#ifndef MOD_AFTERWORLD_MODE
#define MOD_AFTERWORLD_MODE

#include "./gametypes/gametypes.h"
#include "./global.h"
#include "./vehicles.h"
#include "./entity.h"
#include "./gameworld/progress.h"

struct BallModeOptions{
   std::optional<glm::vec3> initialBallPos;
   std::optional<objid> ballId;
};

void startBallMode(objid sceneId);
void endBallMode();

#endif