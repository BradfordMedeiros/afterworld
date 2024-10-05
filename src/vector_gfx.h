#ifndef MOD_AFTERWORLD_VECTOR_GFX
#define MOD_AFTERWORLD_VECTOR_GFX

#include "./util.h"

void drawBloom(objid playerId, objid id, float distance, float bloomAmount);
void drawSphereVecGfx(glm::vec3 position, float radius, glm::vec4 tint);
void drawDebugHitmark(HitObject& hitpoint, objid playerId);

struct WaypointObject {
	std::optional<objid> id;
	bool drawDistance;
	glm::vec4 color;
};
struct Waypoints {
  std::unordered_map<objid, WaypointObject> waypoints;
};
objid addWaypoint(Waypoints& waypoints, std::optional<objid> id);
void removeWaypoint(Waypoints& waypoints, objid id);
void drawWaypoints(Waypoints& waypoints, glm::vec3 playerPos);

#endif