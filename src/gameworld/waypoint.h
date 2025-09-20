#ifndef MOD_AFTERWORLD_WAYPOINT
#define MOD_AFTERWORLD_WAYPOINT

#include "../../../ModEngine/src/cscript/cscript_binding.h"

struct WaypointObject {
	std::optional<objid> targetId;
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