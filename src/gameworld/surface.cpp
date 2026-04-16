#include "./surface.h"

extern CustomApiBindings* gameapi;

extern std::unordered_map<objid, ExtraSurfaceVelocity> extraVelocity;

void drawDebugRaycast(glm::vec3 fromPosition, glm::vec3 toPos, objid playerId);

glm::vec3 getSurfaceVelocityModifiers(objid id){
  if (extraVelocity.find(id) == extraVelocity.end()){
    return glm::vec3(0.f, 0.f, 0.f);
  }

  auto& surface = extraVelocity.at(id);

  auto position = gameapi -> getGameObjectPos(id, true, "[hint] - addSurfaceModifier");
  auto rot = orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
  auto hitpoints = gameapi -> raycast(position, rot, 2.f, std::nullopt);
  for (auto& hitpoint : hitpoints){
    if(hitpoint.id == surface.surfaceId){
      drawDebugRaycast(hitpoint.point, hitpoint.point + 5.f * (hitpoint.normal * glm::vec3(0.f, 0.f, -1.f)), -1);
      return hitpoint.normal * surface.velocity;
    };
  }
  return glm::vec3(0.f, 0.f, 0.f);
}

void addSurfaceModifier(int32_t objId, glm::vec3 amount, int32_t surfaceId){
  extraVelocity[objId] = ExtraSurfaceVelocity {
    .surfaceId = surfaceId,
    .velocity = amount,
  };
}
void removeSurfaceModifier(int32_t obj1, int32_t obj2){
  if (extraVelocity.find(obj1) != extraVelocity.end()){
    if (extraVelocity.at(obj1).surfaceId == obj2){
      extraVelocity.erase(obj1);
    }
  }
  if (extraVelocity.find(obj2) != extraVelocity.end()){
    if (extraVelocity.at(obj2).surfaceId == obj1){
      extraVelocity.erase(obj2);
    }
  }
}
void removeSurfaceModifier(int32_t id){
  extraVelocity.erase(id);
}

void handleSurfaceCollision(int32_t obj1, int32_t obj2){
  auto objAttr1 = getAttrHandle(obj1);
  auto objAttr2 = getAttrHandle(obj2);
  auto obj1ModSpeed = getVec3Attr(objAttr1, "modspeed");
  auto obj2ModSpeed = getVec3Attr(objAttr2, "modspeed");
  if (obj1ModSpeed.has_value() && !obj2ModSpeed.has_value()){
    addSurfaceModifier(obj2, obj1ModSpeed.value(), obj1);
  }else if (!obj1ModSpeed.has_value() && obj2ModSpeed.has_value()){
    addSurfaceModifier(obj1, obj2ModSpeed.value(), obj2);
  }
}
