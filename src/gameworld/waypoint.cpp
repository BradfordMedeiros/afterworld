#include "./waypoint.h"

extern CustomApiBindings* gameapi;

glm::vec2 pointAtSlope(glm::vec2 screenspacePosition, float sizeNdi){
  float slopeY = screenspacePosition.y;
  float slopeX = screenspacePosition.x;
  float slope = slopeY / slopeX;
  float inverseSlope = slopeX / slopeY;

  float halfWidth = sizeNdi * 0.5f;
  float halfHeight = sizeNdi * 0.5f;

  // left and right side
  if (slope > 0 && slope < 1 && screenspacePosition.x > 0){  // top right on x side
    auto yValue = slope * 1.f;
    return glm::vec2(1.f, yValue) - halfWidth;
  }
  if (slope < 0 && slope > -1 && screenspacePosition.x < 0){  // top left on -x side
    auto yValue = slope * -1.f;
    return glm::vec2(-1.f, yValue) + halfWidth;
  }
  if (slope > 0 && slope < 1 && screenspacePosition.x < 0){  // bottom left on -x side
    auto yValue = slope * -1.f;
    return glm::vec2(-1.f, yValue) + halfWidth;
  }
  if (slope < 0 && slope > -1 && screenspacePosition.x > 0){  // bottom right on x side
    auto yValue = slope * 1.f;
    return glm::vec2(1.f, yValue) - halfWidth;
  }

  // top and bottom side
  if (slope > 1 && screenspacePosition.x > 0){  // upper right side
    auto xValue = inverseSlope * 1.f;
    return glm::vec2(xValue, 1.f) - halfHeight;
  }
  if (slope > 1 && screenspacePosition.x < 0){  // bottom left side
    auto xValue = inverseSlope * -1.f;
    return glm::vec2(xValue, -1.f) + halfHeight;
  }

  if (slope < 1 && screenspacePosition.x > 0){  // bottom right side
    auto xValue = inverseSlope * -1.f;
    return glm::vec2(xValue, -1.f) + halfHeight;
  }
  if (slope < 1 && screenspacePosition.x < 0){  // top left side
    auto xValue = inverseSlope * 1.f;
    return glm::vec2(xValue, 1.f) - halfHeight;
  }
  return glm::vec2(0.f, 0.f);
}

void drawWaypoint(glm::vec3 position, glm::vec3 playerPos, bool drawDistance, glm::vec4 color, std::optional<float> percentage){
  auto ndiPosition = gameapi -> positionToNdi(position);
  // ndi position is basically ndi intersects with screenspace
  // when it's behind us, we could just draw it, and would be accurate. 
  // but instead -1  makes it so you reflect which is path to the object
  // the max length means its guaranteed to fall outside the screen (which would otherwise show on the opposite side)
  if (ndiPosition.z < 0){ 
    auto maxLength = glm::max(1 / glm::abs(ndiPosition.x), 1 / glm::abs(ndiPosition.y));  
    ndiPosition.x *= -1 * maxLength;
    ndiPosition.y *= -1 * maxLength;
  }
  auto screenspacePosition = glm::vec2(ndiPosition.x, ndiPosition.y);
  if (screenspacePosition.x > 1.f || screenspacePosition.x < -1.f || screenspacePosition.y > 1.f || screenspacePosition.y < -1.f){
    auto point = pointAtSlope(screenspacePosition, 0.02f);
    gameapi -> drawRect(point.x, point.y, 0.02f, 0.02f, false, color, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
  }else{
    gameapi -> drawRect(screenspacePosition.x, screenspacePosition.y, 0.02f, 0.02f, false, color, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
    if (drawDistance){
      int distance = glm::distance(position, playerPos);
      gameapi -> drawText(std::to_string(distance), screenspacePosition.x, screenspacePosition.y, 10.f, false, color, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);      
    }
    if (percentage.has_value()){
      gameapi -> drawRect(screenspacePosition.x, screenspacePosition.y, 0.2f, 0.04f, false, glm::vec4(0.f, 0.f, 0.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
      gameapi -> drawRect(screenspacePosition.x, screenspacePosition.y, 0.2f * percentage.value(), 0.04f, false, color, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
    }
  }
}
void drawWaypoints(Waypoints& waypoints, glm::vec3 playerPos){
  for (auto &[id, waypoint] : waypoints.waypoints){
    if (waypoint.targetId.has_value()){
      auto objectExists = gameapi -> gameobjExists(waypoint.targetId.value());
      if (objectExists){
        auto position = gameapi -> getGameObjectPos(waypoint.targetId.value(), true, "[gamelogic] drawWaypoints - get waypoint locations");
        drawWaypoint(position, playerPos, waypoint.drawDistance, waypoint.color, waypoint.percentage);
      }
    }
  }
}

void addWaypoint(Waypoints& waypoints, objid id, std::optional<objid> waypointId){
  modassert(waypoints.waypoints.find(id) == waypoints.waypoints.end(), "waypoint id already defined");
  waypoints.waypoints[id] = WaypointObject {
    .targetId = waypointId,
    .drawDistance = true,
    .color = glm::vec4(0.f, 1.f, 0.f, 0.3f),
    .percentage = 0.3f,
  };
}

void removeWaypoint(Waypoints& waypoints, objid id){
  waypoints.waypoints.erase(id);
}

void updateHealth(Waypoints& waypoints, objid id, std::optional<float> health){
  if (waypoints.waypoints.find(id) != waypoints.waypoints.end()){
    waypoints.waypoints.at(id).percentage = health;
  }
}

