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
	std::optional<float> percentage;
};
struct Waypoints {
  std::unordered_map<objid, WaypointObject> waypoints;
};
void addWaypoint(Waypoints& waypoints, objid id, std::optional<objid> waypointId);
void removeWaypoint(Waypoints& waypoints, objid id);
void updateHealth(Waypoints& waypoints, objid id, std::optional<float> health);

void drawWaypoints(Waypoints& waypoints, glm::vec3 playerPos);

#endif