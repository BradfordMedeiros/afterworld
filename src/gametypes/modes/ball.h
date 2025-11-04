#ifndef MOD_AFTERWORLD_GAMETYPES_BALL
#define MOD_AFTERWORLD_GAMETYPES_BALL

#include "../common.h"

struct BallModeOptions{
   int testNumber;

   std::function<void()> setPlayerControl;
};

GameTypeInfo getBallMode();

#endif 