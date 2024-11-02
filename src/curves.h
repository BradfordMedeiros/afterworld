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

void attachToCurve(objid entityId, objid railId, glm::vec3 position);
void unattachToCurve(objid entityId);
void handleEntitiesOnRails(objid owner, objid playerId);

#endif