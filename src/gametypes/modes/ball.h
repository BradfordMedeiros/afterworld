#ifndef MOD_AFTERWORLD_GAMETYPES_BALL
#define MOD_AFTERWORLD_GAMETYPES_BALL

#include "../common.h"

struct BallModeOptions{
   int testNumber;

   std::function<void()> setPlayerControl;
   std::function<void(bool)> changeUi;
   std::function<void(std::optional<float>)> showTimeElapsed;

   std::function<void()> setLevelFinished;
};

GameTypeInfo getBallMode();

#endif 