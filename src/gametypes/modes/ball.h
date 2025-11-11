#ifndef MOD_AFTERWORLD_GAMETYPES_BALL
#define MOD_AFTERWORLD_GAMETYPES_BALL

#include "../common.h"

struct BallModeOptions{
   int testNumber;

   std::function<std::optional<objid>()> getBallId;
   std::function<void(std::function<void()>)> setPlayerControl;
   std::function<void(bool)> changeUi;
   std::function<void(std::optional<float>)> showTimeElapsed;
   std::function<void()> setLevelFinished;

   std::optional<glm::vec3> initialBallPos;
   std::optional<objid> ballId;
};

GameTypeInfo getBallMode();

void createBallObj(objid sceneId);
void createLevelObj(objid sceneId, std::string ballLevel);


#endif 