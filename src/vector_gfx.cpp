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
  gameapi -> drawLine2D(glm::vec3(0.f, marketInner, 0.f), glm::vec3(0.f, markerOuter, 0.f), false, reticleColor, std::nullopt, true, std::nullopt, std::nullopt);
  gameapi -> drawLine2D(glm::vec3(0.f, -1 * marketInner, 0.f), glm::vec3(0.f, -1 * markerOuter, 0.f), false, reticleColor, std::nullopt, true, std::nullopt, std::nullopt);
  gameapi -> drawLine2D(glm::vec3(marketInner, 0.f, 0.f), glm::vec3(markerOuter, 0.f, 0.f), false, reticleColor, std::nullopt, true, std::nullopt, std::nullopt);
  gameapi -> drawLine2D(glm::vec3(-1 * marketInner, 0.f, 0.f), glm::vec3(-1 * markerOuter, 0.f, 0.f), false, reticleColor, std::nullopt, true, std::nullopt, std::nullopt);
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

float debugMarkerHitlength = 2;
void drawDebugHitmark(HitObject& hitpoint, objid playerId){
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
