#include "./triggerzone.h"

extern CustomApiBindings* gameapi;

extern std::unordered_map<objid, std::set<objid>> triggerZoneIdToElements;

bool isControlledVehicle(int vehicleId);
void triggerMovement(std::string trigger, std::optional<int> railIndex);
void triggerColor(std::string trigger);
void setTempCamera(std::optional<objid> camera, int playerIndex);

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

