#ifndef MOD_AFTERWORLD_GAMETYPES_TARGET_KILL
#define MOD_AFTERWORLD_GAMETYPES_TARGET_KILL

#include "./common.h"

struct TargetKillMode {
  int numTargets;
  float startTime;
  float durationSeconds;
};

GameTypeInfo getTargetKill();

#endif 