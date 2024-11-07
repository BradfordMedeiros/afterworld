#ifndef MOD_AFTERWORLD_CURVES
#define MOD_AFTERWORLD_CURVES

#include "../../ModEngine/src/cscript/cscript_binding.h"

struct LinePoints {
  std::vector<glm::vec3> points;
};

struct RaceData {
  int currentPosition;
  int maxPosition;
  float percentageToNext;
  bool complete;
};

struct EntityOnRail {
  objid railId;
  bool direction;
};

void attachToCurve(objid entityId, objid railId);
void unattachToCurve(objid entityId);
void maybeReverseDirection(objid entityId);

void addToRace(objid entityId);
void removeFromRace(objid entityId);

void handleEntitiesOnRails(objid owner, objid sceneId);

#endif