#include "./collision.h"

extern CustomApiBindings* gameapi;
extern std::unordered_map<objid, Spawnpoint> managedSpawnpoints;
void doDamageMessage(objid targetId, float damage);
void doDialogMessage(std::string& value);

void handleInteract(objid gameObjId){
  auto objAttr = getAttrHandle(gameObjId);
  auto chatNode = getStrAttr(objAttr, "chatnode");
  if (chatNode.has_value()){
    doDialogMessage(chatNode.value());
  }
  auto triggerSwitch = getStrAttr(objAttr, "trigger-switch");
  if (triggerSwitch.has_value()){
    gameapi -> sendNotifyMessage("switch", triggerSwitch.value());
  }
}


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
  auto switchEnter1 = getAttr(objAttr1, attrForValue.c_str());
  auto switchEnter1Key = getStrAttr(objAttr1, attrForKey.c_str());
  auto switchRemove1 = getStrAttr(objAttr1, "switch-remove");
  auto passedFilter1 = passesSwitchFilter(objAttr1, obj2);
  if (switchEnter1.has_value() && passedFilter1){
    //std::cout << "race publishing 1: " << switchEnter1.value() << std::endl;
    auto key = switchEnter1Key.has_value() ? switchEnter1Key.value() : "switch";
    std::cout << "handle collision: " << key << ", " << print(switchEnter1.value()) << std::endl;

    gameapi -> sendNotifyMessage(key, switchEnter1.value());
    if (switchRemove1.has_value() && switchRemove1.value() == removeKey){
      gameapi -> removeByGroupId(obj1);
    }
  }

  auto objAttr2 = getAttrHandle(obj2);
  auto switchEnter2 = getAttr(objAttr2, attrForValue.c_str());
  auto switchEnter2Key = getStrAttr(objAttr2, attrForKey.c_str());
  auto switchRemove2 = getStrAttr(objAttr2, "switch-remove");
  auto passedFilter2 = passesSwitchFilter(objAttr2, obj1);
  if (switchEnter2.has_value() && passedFilter2){
    //std::cout << "race publishing 2: " << switchEnter2.value() << std::endl;
    auto key = switchEnter2Key.has_value() ? switchEnter2Key.value() : "switch";
    std::cout << "handle collision:2 " << key << ", " << print(switchEnter2.value()) << std::endl;
    
    gameapi -> sendNotifyMessage(key, switchEnter2.value());
    if (switchRemove2.has_value() && switchRemove2.value() == removeKey){
      gameapi -> removeByGroupId(obj2);
    }
  }
}

void handleDamageCollision(objid obj1, objid obj2){
  modlog("damage collision: ", gameapi -> getGameObjNameForId(obj1).value() + ", " + gameapi -> getGameObjNameForId(obj2).value());
  
  {
    auto objAttr1 = getAttrHandle(obj1);
    auto damageAmount = getFloatAttr(objAttr1, "touch-damage");
    if (damageAmount.has_value()){
      doDamageMessage(obj2, damageAmount.value());
      gameapi -> removeByGroupId(obj1);
    }
  }
   
  {
    auto objAttr2 = getAttrHandle(obj2);
    auto damageAmount2 = getFloatAttr(objAttr2, "touch-damage");
    if (damageAmount2.has_value()){
      doDamageMessage(obj1, damageAmount2.value());
      gameapi -> removeByGroupId(obj2);
    }
  }
}

void handleMomentumCollision(objid obj1, objid obj2, glm::vec3 position, glm::quat direction, float force){
  static float lastForce = 0.f;
  if (force > 1){
    lastForce  = force;
  }


  static unsigned int forceStat = gameapi -> stat("last-force");
  gameapi -> logStat(forceStat, lastForce);

  if (force > 50){
    {
      float volume = 1.f;  // should adjust based on force, how much? 
      playGameplayClipById(getManagedSounds().landSoundObjId.value(), volume * force / 10.f, position);


      auto attr = getAttrHandle(obj1);
      auto collideDamage = getFloatAttr(attr, "collide_damage"); 
      if (collideDamage.has_value()){
        doDamageMessage(obj1, collideDamage.value());   
      }
    }
    {
      auto attr = getAttrHandle(obj2);
      auto collideDamage = getFloatAttr(attr, "collide_damage"); 
      if (collideDamage.has_value()){
        doDamageMessage(obj2, collideDamage.value());   
      }
    }

  }


  // i think could calculate the area (which maybe just simplify from volume), and that gives a rough value for pressure
  // then from that pressure, 
  // and then use some sort of that value, maybe mass?  and some coefficient?  to dtermine if should break
  // can also be used to inflict damage on another object
}

void handleBouncepadCollision(objid obj1, objid obj2, glm::vec3 normal){

  {
    glm::vec3 oppositeNormal(normal.x * -1, normal.y * -1, normal.z * -1);
    auto attr = getAttrHandle(obj1);
    auto bounceAmount = getVec3Attr(attr, "bounce");
    if (bounceAmount.has_value()){
      auto impulse = orientationFromPos(glm::vec3(0.f, 0.f, 0.f), oppositeNormal) * bounceAmount.value();
      gameapi -> applyImpulse(obj2, impulse);
    }    
  }

  {
    auto attr = getAttrHandle(obj2);
    auto bounceAmount = getVec3Attr(attr, "bounce");
    if (bounceAmount.has_value()){
      auto impulse = orientationFromPos(glm::vec3(0.f, 0.f, 0.f), normal) * bounceAmount.value();
      gameapi -> applyImpulse(obj1, impulse);
    }    
  }
}



bool isPlayer(objid id){
  auto playerAttr = getSingleAttr(id, "player");
  return playerAttr.has_value() && playerAttr.value() == "true";
}
bool isPickup(objid id){
  auto playerAttr = getSingleAttr(id, "pickup");
  return playerAttr.has_value();
}
void tryPickupItem(objid gameObjId, objid playerId){
  objid inventory = playerId;

  auto objAttr = getAttrHandle(gameObjId);
  auto pickup = getStrAttr(objAttr, "pickup");
  if (pickup.has_value()){
    auto pickupTrigger = getStrAttr(objAttr, "pickup-trigger");
    auto pickupQuantity = getFloatAttr(objAttr, "pickup-amount");
    auto pickupType = getStrAttr(objAttr, "pickup-type");
    auto pickupRemove = getStrAttr(objAttr, "pickup-remove");
    auto quantityAmount = pickupQuantity.has_value() ? pickupQuantity.value() : 1.f;

    auto oldItemCount = currentItemCount(inventory, pickup.value());
    auto newItemCount = (pickupType.has_value() && pickupType.value() == "replace") ? quantityAmount : (oldItemCount + quantityAmount);
    updateItemCount(inventory, pickup.value(), newItemCount);

    if (!pickupRemove.has_value()){
      gameapi -> removeByGroupId(gameObjId);
    }else if (pickupRemove.value() == "prefab"){
      auto prefabRootId = gameapi -> prefabId(gameObjId);
      modassert(prefabRootId.has_value(), "inventory remove prefab, but object id is not a prefab type");
      gameapi -> removeByGroupId(prefabRootId.value());
    }
    
    if (pickupTrigger.has_value()){
      ItemAcquiredMessage itemAcquiredMessage {
        .targetId = playerId,
        .amount = static_cast<int>(newItemCount),
      };
      gameapi -> sendNotifyMessage(pickupTrigger.value(), itemAcquiredMessage);
    }
  }
}
void handleInventoryOnCollision(int32_t obj1, int32_t obj2){
  auto obj1IsPlayer = isPlayer(obj1);
  auto obj2IsPlayer = isPlayer(obj2);
  auto obj1IsPickup = isPickup(obj1);
  auto obj2IsPickup = isPickup(obj2);
  if (obj1IsPlayer && obj2IsPickup){
    tryPickupItem(obj2, obj1);
  }else if (obj2IsPlayer && obj1IsPickup){
    tryPickupItem(obj1, obj2);
  }
}


void handleSpawnCollision(int32_t obj1, int32_t obj2, std::optional<objid> activePlayerId){
  if(!activePlayerId.has_value()){
    return;
  }
  auto playerId = activePlayerId.value();
  if (obj2 == playerId){
    auto objAttr = getAttrHandle(obj1);
    auto spawnPointTag = getStrAttr(objAttr, "spawn-trigger");
    if (spawnPointTag.has_value()){
      spawnFromAllSpawnpoints(managedSpawnpoints, spawnPointTag.value().c_str());
      gameapi -> removeByGroupId(obj1);
    }
  }else if (obj1 == playerId){
    auto objAttr = getAttrHandle(obj2);
    auto spawnPointTag = getStrAttr(objAttr, "spawn-trigger"); 
    if (spawnPointTag.has_value()){
      spawnFromAllSpawnpoints(managedSpawnpoints, spawnPointTag.value().c_str());
      gameapi -> removeByGroupId(obj2);
    }
  }
}

void showTriggerVolumes(bool showTriggerVolumes){

}