#ifndef MOD_AFTERWORLD_MODE
#define MOD_AFTERWORLD_MODE

#include "./gametypes/gametypes.h"
#include "./global.h"
#include "./vehicles.h"
#include "./entity.h"
#include "./gameworld/progress.h"
#include "./orbs.h"

struct BallModeOptions{
   std::optional<glm::vec3> initialBallPos;
   std::optional<objid> ballId;
};

void startBallMode(objid sceneId);
void endBallMode();
void ballModeLevelSelect(std::optional<int> targetIndex);

void startIntroMode(objid sceneId);
void endIntroMode();

#endif