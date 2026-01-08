#ifndef MOD_AFTERWORLD_CURVES
#define MOD_AFTERWORLD_CURVES

#include "../../ModEngine/src/cscript/cscript_binding.h"

struct ManagedRailMovement {
  objid railId;
  glm::vec3 initialObjectPos;

  float initialStartTime;
};
struct LinePoints {
  objid railId;
  std::string railName;
  std::vector<glm::vec3> points;
  std::vector<int> indexs;
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

struct RailNode {
  std::string rail;
  int railIndex;
  glm::vec3 point;
};
void addRails(objid ownerId, std::vector<RailNode>& railNodes);
void removeRails(objid ownerId);
std::optional<objid> railIdForName(std::string name);

void drawAllCurves(objid ownerId);

void attachToCurve(objid entityId, objid railId, bool direction);
void unattachToCurve(objid entityId);
bool isAttachedToCurve(objid entityId);
void maybeReverseDirection(objid entityId);

void addToRace(objid entityId);
void removeFromRace(objid entityId);

void handleEntitiesOnRails(objid owner, objid sceneId);
void handleEntitiesRace();

struct NearbyRail {
  objid id;
  bool direction;
};

std::optional<NearbyRail> nearbyRail(glm::vec3 position, glm::vec3 forwardDir);

#endif