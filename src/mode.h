#ifndef MOD_AFTERWORLD_MODE
#define MOD_AFTERWORLD_MODE

#include "./gametypes/gametypes.h"
#include "./global.h"
#include "./core/vehicles/vehicles.h"
#include "./entity.h"
#include "./gameworld/progress.h"
#include "./orbs.h"
#include "./interaction/cutscene.h"
#include "./collision.h"

struct BallModeOptions{
   std::optional<glm::vec3> initialBallPos;
   std::optional<objid> ballId;
   bool didLose;
   bool shouldReset;
   bool didReset;

};

void startBallMode(objid sceneId);
void endBallMode();

void startIntroMode(objid sceneId);
void endIntroMode();

void onModeOrbSelect(std::vector<OrbSelection>& selectedOrbs);

void ballModeNewGame();
void ballModeLevelSelect();

#endif