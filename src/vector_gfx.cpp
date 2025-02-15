#include "./vector_gfx.h"

extern CustomApiBindings* gameapi;

glm::vec3 calculatePoint(float radians, float radius, glm::quat orientation){
  float x = glm::cos(radians) * radius;
  float y = glm::sin(radians) * radius;
  return orientation * glm::vec3(x, y, 0.f);
}
const int CIRCLE_RESOLUTION = 20;
void drawCircle(objid id, glm::vec3 pos, float radius, glm::quat orientation, std::optional<glm::vec4> tint = std::nullopt){
  auto lastPoint = calculatePoint(0, radius, orientation);
  //std::cout << std::endl;
  for (int i = 1; i <= CIRCLE_RESOLUTION; i++){
    auto radians = i * ((2 * MODPI) / static_cast<float>(CIRCLE_RESOLUTION));
    auto newPoint = calculatePoint(radians, radius, orientation);
    //std::cout << "radians = " << radians << ", new point is: " << print(newPoint) << std::endl;
    gameapi -> drawLine(lastPoint + pos,  newPoint + pos, false, id, tint, std::nullopt, std::nullopt);
    lastPoint = newPoint;
  } 
}

const glm::vec4 reticleColor(1.f, 1.f, 1.f, 0.5f);
const float markerLineLength = 0.01f;

bool shouldDrawMarkers = true;
void drawMarkers(objid id, glm::vec3 pos, float radius, glm::quat orientation){
  float marketInner = radius * 10.f;
  float markerOuter = radius * 10.f + markerLineLength; //
  gameapi -> drawLine2D(glm::vec3(0.f, marketInner, 0.f), glm::vec3(0.f, markerOuter, 0.f), false, reticleColor, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
  gameapi -> drawLine2D(glm::vec3(0.f, -1 * marketInner, 0.f), glm::vec3(0.f, -1 * markerOuter, 0.f), false, reticleColor, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
  gameapi -> drawLine2D(glm::vec3(marketInner, 0.f, 0.f), glm::vec3(markerOuter, 0.f, 0.f), false, reticleColor, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
  gameapi -> drawLine2D(glm::vec3(-1 * marketInner, 0.f, 0.f), glm::vec3(-1 * markerOuter, 0.f, 0.f), false, reticleColor, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
}

// draw a circle at a distance from the player with a certain radius
// this is independent of fov, and should be
void drawBloom(objid playerId, objid id, float distance, float bloomAmount){

  // i'd rather do this on a screen only texture
  modassert(distance < 0, "distance must be negative (forward -z)");
  auto mainobjPos = gameapi -> getGameObjectPos(playerId, true);
  auto mainobjRot = gameapi -> getGameObjectRotation(playerId, true);
  auto toPos = mainobjPos + mainobjRot * glm::vec3(0.f, 0.f, distance);
  gameapi -> drawLine(mainobjPos, toPos, false, id, reticleColor, std::nullopt, std::nullopt);
  
  if (shouldDrawMarkers){
    drawMarkers(id, toPos, bloomAmount, mainobjRot);
  }else{
    float radius = glm::max(0.002f, bloomAmount);
    drawCircle(id, toPos, radius, mainobjRot);
  }
}


void drawSphereVecGfx(glm::vec3 position, float radius, glm::vec4 tint){
  drawCircle(gameapi -> rootSceneId(), position, radius, gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f)), tint);
  drawCircle(gameapi -> rootSceneId(), position, radius, gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f)), tint);
  drawCircle(gameapi -> rootSceneId(), position, radius, gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f)), tint);
}

///////// debug //////////////////

bool drawDebugVector = false;

void setDrawDebugVector(bool shouldDrawDebugVector){
  drawDebugVector = shouldDrawDebugVector;
}

float debugMarkerHitlength = 2;
void drawDebugHitmark(HitObject& hitpoint, objid playerId){
  if (!drawDebugVector){
    return;
  }
  gameapi -> drawLine(
    hitpoint.point + glm::vec3(0.f, -1.f * debugMarkerHitlength,  0.f),
    hitpoint.point + glm::vec3(0.f, 1.f * debugMarkerHitlength,  0.f),
    true, playerId, std::nullopt,  std::nullopt, std::nullopt
  );
  gameapi -> drawLine(
    hitpoint.point + glm::vec3(-1.f * debugMarkerHitlength, 0.f, 0.f),
    hitpoint.point + glm::vec3(1.f * debugMarkerHitlength, 0.f, 0.f),
    true, playerId, std::nullopt,  std::nullopt, std::nullopt
  );
  gameapi -> drawLine(
    hitpoint.point + glm::vec3(0.f, 0.f, -1.f * debugMarkerHitlength),
    hitpoint.point + glm::vec3(0.f, 0.f, 1.f * debugMarkerHitlength),
    true, playerId, std::nullopt,  std::nullopt, std::nullopt
  );

  gameapi -> drawLine(
    hitpoint.point,
    hitpoint.point + (10.f * (hitpoint.normal * glm::vec3(0.f, 0.f, -1.f))),
    true, playerId, glm::vec4(1.f, 0.f, 0.f, 1.f),  std::nullopt, std::nullopt
  );
}

void drawDebugRaycast(glm::vec3 fromPosition, glm::vec3 toPos, objid playerId){
  if (!drawDebugVector){
    return;
  }
  gameapi -> drawLine(
    fromPosition,
    toPos,
    true, playerId, std::nullopt,  std::nullopt, std::nullopt
  );
}

//////////////// waypoint code ///////////////////////

void addWaypoint(Waypoints& waypoints, objid id, std::optional<objid> waypointId){
  modassert(waypoints.waypoints.find(id) == waypoints.waypoints.end(), "waypoint id already defined");
  waypoints.waypoints[id] = WaypointObject {
    .id = waypointId,
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
    if (waypoint.id.has_value()){
      auto objectExists = gameapi -> gameobjExists(waypoint.id.value());
      if (objectExists){
        auto position = gameapi -> getGameObjectPos(waypoint.id.value(), true);
        drawWaypoint(position, playerPos, waypoint.drawDistance, waypoint.color, waypoint.percentage);
      }
    }
  }
}