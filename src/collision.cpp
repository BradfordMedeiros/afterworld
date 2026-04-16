#include "./collision.h"

extern CustomApiBindings* gameapi;

extern std::unordered_map<objid, std::set<objid>> triggerZoneIdToElements;

bool isControlledVehicle(int vehicleId);
void triggerMovement(std::string trigger, std::optional<int> railIndex);
void triggerColor(std::string trigger);
void setTempCamera(std::optional<objid> camera, int playerIndex);

bool passesSwitchFilter(ObjectAttrHandle& handle, objid otherObjId){
  auto switchFilter = getStrAttr(handle, "switch-filter");
  if (!switchFilter.has_value()){
    return true;
  }
  auto otherAttr = getAttrHandle(otherObjId);
  auto filterValue = getAttr(otherAttr, switchFilter.value().c_str());
  return filterValue.has_value();
}
void handleCollision(objid obj1, objid obj2, std::string attrForValue, std::string attrForKey, std::string removeKey){
  modlog("main collision: ", gameapi -> getGameObjNameForId(obj1).value() + ", " + gameapi -> getGameObjNameForId(obj2).value());
  auto objAttr1 =  getAttrHandle(obj1);
  std::optional<AttributeValue> switchEnter1 = getAttr(objAttr1, attrForValue.c_str());
  auto switchEnter1Key = getStrAttr(objAttr1, attrForKey.c_str());
  auto switchRemove1 = getStrAttr(objAttr1, "switch-remove");
  auto passedFilter1 = passesSwitchFilter(objAttr1, obj2);
  if (switchEnter1.has_value() && passedFilter1){
    //std::cout << "race publishing 1: " << switchEnter1.value() << std::endl;
    auto key = switchEnter1Key.has_value() ? switchEnter1Key.value() : "switch";
    std::cout << "handle collision: " << key << ", " << print(switchEnter1.value()) << ", remove = " << print(switchRemove1) << std::endl;

    gameapi -> sendNotifyMessage(key, switchEnter1.value());
    if (switchRemove1.has_value() && switchRemove1.value() == removeKey){
      gameapi -> removeByGroupId(obj1);
    }
  }

  auto objAttr2 = getAttrHandle(obj2);
  std::optional<AttributeValue> switchEnter2 = getAttr(objAttr2, attrForValue.c_str());
  auto switchEnter2Key = getStrAttr(objAttr2, attrForKey.c_str());
  auto switchRemove2 = getStrAttr(objAttr2, "switch-remove");
  auto passedFilter2 = passesSwitchFilter(objAttr2, obj1);
  if (switchEnter2.has_value() && passedFilter2){
    //std::cout << "race publishing 2: " << switchEnter2.value() << std::endl;
    auto key = switchEnter2Key.has_value() ? switchEnter2Key.value() : "switch";
    std::cout << "handle collision:2 " << key << ", " << print(switchEnter2.value()) << ", remove = " << print(switchRemove2) << std::endl;
    
    gameapi -> sendNotifyMessage(key, switchEnter2.value());
    if (switchRemove2.has_value() && switchRemove2.value() == removeKey){
      gameapi -> removeByGroupId(obj2);
    }
  }
}

void addToTriggerZone(objid id, objid triggerZone){
  //modassert(false, std::string("addToTriggerZone: ") + idName + ", " + triggerName);
  if (triggerZoneIdToElements.find(triggerZone) == triggerZoneIdToElements.end()){
    triggerZoneIdToElements[triggerZone] = {};
  }
  triggerZoneIdToElements.at(triggerZone).insert(id);

  auto idName = gameapi -> getGameObjNameForId(id).value();
  auto triggerName = gameapi -> getGameObjNameForId(triggerZone).value();
  std::cout << "triggerbound: " << std::string("addToTriggerZone: ") + idName + ", " + triggerName << std::endl;
}
void onExitTriggerZone(objid triggerZone){
  auto objAttr = getAttrHandle(triggerZone);
  auto cameraTarget = getStrAttr(objAttr, "camera_target");
  if (cameraTarget.has_value()){
    setTempCamera(std::nullopt, 0);
  }
}
void removeFromTriggerZone(objid id, objid triggerZone){
  if (triggerZoneIdToElements.find(triggerZone) != triggerZoneIdToElements.end()){
    auto numRemoved = triggerZoneIdToElements.at(triggerZone).erase(id);

    auto idName = gameapi -> getGameObjNameForId(id).value();
    auto triggerName = gameapi -> getGameObjNameForId(triggerZone).value();
    if (numRemoved > 0){
      onExitTriggerZone(triggerZone);
      std::cout << "triggerbound: " << std::string("removeFromTriggerZone: ") + idName + ", " + triggerName << std::endl;
    }
  }
}
void handleOnTriggerExit(objid obj1, objid obj2){
  removeFromTriggerZone(obj1, obj2);
  removeFromTriggerZone(obj2, obj1);
}

void handleOnTriggerRemove(objid id){
  auto triggerName = gameapi -> getGameObjNameForId(id).value();

  auto elementsRemoved = triggerZoneIdToElements.erase(id);
  if (elementsRemoved > 0){
    std::cout << "triggerbound: " << std::string("handleOnTriggerRemove: ") + triggerName << std::endl;
  }

  for (auto& [triggerId, elements] : triggerZoneIdToElements){
    auto innerElementsRemoved = elements.erase(id);
    if (innerElementsRemoved){
      onExitTriggerZone(triggerId);
      std::cout << "triggerbound: " << std::string("handleOnTriggerRemove: ") + triggerName << std::endl;
    }
  }
}

void handleOnTriggerEnter(objid obj1, objid obj2){
  {
    auto objAttr = getAttrHandle(obj1);
    auto trigger = getStrAttr(objAttr, "trigger_bound");
    if (trigger.has_value()){
      addToTriggerZone(obj2, obj1);
    }
  }
  {
    auto objAttr = getAttrHandle(obj2);
    auto trigger = getStrAttr(objAttr, "trigger_bound");
    if (trigger.has_value()){
      addToTriggerZone(obj1, obj2);
    }
  }
}

void handleTriggerZone(int32_t obj1, int32_t obj2){
  if (isControlledVehicle(obj1)){
    auto objAttr = getAttrHandle(obj2);
    auto triggerZone = getStrAttr(objAttr, "trigger_zone");
    if (triggerZone.has_value()){
      auto triggerData = getFloatAttr(objAttr, "trigger_data");
      if (triggerData.has_value()){
        auto triggerValue  = static_cast<int>(triggerData.value());
        triggerMovement(triggerZone.value(), triggerValue);
        triggerColor(triggerZone.value());
      }else{
        triggerMovement(triggerZone.value(), std::nullopt);
        triggerColor(triggerZone.value());
      }
    }

    auto triggerEvent = getStrAttr(objAttr, "trigger_event");
    if (triggerEvent.has_value()){
      auto triggerEventValue = getStrAttr(objAttr, "trigger_event_value");
      if (triggerEventValue.has_value()){
        gameapi -> sendNotifyMessage(triggerEvent.value(), triggerEventValue.value());
      }else{
        gameapi -> sendNotifyMessage(triggerEvent.value(), std::string(""));
      }
    }

    auto cameraTarget = getStrAttr(objAttr, "camera_target");
    if (cameraTarget.has_value()){
      auto ids = gameapi -> getObjectsByAttr("cameratag", cameraTarget.value(), std::nullopt);
      modassert(ids.size() > 0, "no ids found for this camera tag");
      setTempCamera(ids.at(0), 0);
    }

  }else if (isControlledVehicle(obj2)){
    auto objAttr = getAttrHandle(obj1);
    auto triggerZone = getStrAttr(objAttr, "trigger_zone");
    if (triggerZone.has_value()){
      auto triggerData = getFloatAttr(objAttr, "trigger_data");
      if (triggerData.has_value()){
        auto triggerValue  = static_cast<int>(triggerData.value());
        triggerMovement(triggerZone.value(), triggerValue);
        triggerColor(triggerZone.value());
      }else{
        triggerMovement(triggerZone.value(), std::nullopt);
        triggerColor(triggerZone.value());
      }
    }
    auto triggerEvent = getStrAttr(objAttr, "trigger_event");
    if (triggerEvent.has_value()){
      auto triggerEventValue = getStrAttr(objAttr, "trigger_event_value");
      if (triggerEventValue.has_value()){
        gameapi -> sendNotifyMessage(triggerEvent.value(), triggerEventValue.value());
      }else{
        gameapi -> sendNotifyMessage(triggerEvent.value(), std::string(""));
      }
    }

    auto cameraTarget = getStrAttr(objAttr, "camera_target");
    if (cameraTarget.has_value()){
      auto ids = gameapi -> getObjectsByAttr("cameratag", cameraTarget.value(), std::nullopt);
      modassert(ids.size() > 0, "no ids found for this camera tag");
      setTempCamera(ids.at(0), 0);
    }

  }
}


struct ExtraSurfaceVelocity {
  objid surfaceId;
  glm::vec3 velocity;
};
std::unordered_map<objid, ExtraSurfaceVelocity> extraVelocity;
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



