#ifndef MOD_AFTERWORLD_CURVES
#define MOD_AFTERWORLD_CURVES

#include "../../ModEngine/src/cscript/cscript_binding.h"

struct LinePoints {
  std::vector<glm::vec3> points;
};

void drawCurve(LinePoints& line, glm::vec3 point, objid owner);
void drawAllCurves(objid owner);

void attachToCurve(objid entityId, objid railId);
void unattachToCurve(objid entityId);
void handleEntitiesOnRails(objid owner, objid playerId);

#endif