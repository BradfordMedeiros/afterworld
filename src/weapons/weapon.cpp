#include "./weapon.h"

extern CustomApiBindings* gameapi;

struct Weapons {
  std::optional<objid> playerId;
  bool isHoldingLeftMouse;
  bool isHoldingRightMouse;
  bool fireOnce;
  float selectDistance;

  GunInstance weaponValues;

  glm::vec2 lookVelocity;
  glm::vec3 movementVec;
  std::optional<objid> heldItem;
};

std::string weaponsToString(Weapons& weapons){
  std::string str;
  str += std::string("isHoldingLeftMouse: ") + (weapons.isHoldingLeftMouse ? "true" : "false") + "\n";
  str += std::string("isHoldingRightMouseo: ") + (weapons.isHoldingRightMouse ? "true" : "false") + "\n";
  return str;
}

void reloadTraitsValues(Weapons& weapons){
  auto traitQuery = gameapi -> compileSqlQuery("select select-distance from traits where profile = ?", { "default" });
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(traitQuery, &validSql);
  modassert(validSql, "error executing sql query");
  weapons.selectDistance = floatFromFirstSqlResult(result, 0);
}

// Should interpolate.  Looks better + prevent clipping bugs
// Might be interesting to incorporate things like mass and stuff
void handlePickedUpItem(Weapons& weapons){
  if (!weapons.heldItem.has_value()){
    return;
  }

  auto playerPos = gameapi -> getGameObjectPos(weapons.playerId.value(), true);
  auto playerRotation = gameapi -> getGameObjectRotation(weapons.playerId.value(), true);  // maybe this should be the gun rotation instead, problem is the offsets on the gun
  glm::vec3 distanceFromPlayer = glm::vec3(0.f, 0.f, -5.f); 
  auto slightlyInFrontOfPlayer = gameapi -> moveRelativeVec(playerPos, playerRotation, distanceFromPlayer);

  // old object position
  auto oldItemPos = gameapi -> getGameObjectPos(weapons.heldItem.value(), true);
  auto towardPlayerView = slightlyInFrontOfPlayer - oldItemPos;
  auto distance = glm::length(towardPlayerView);
  auto direction = glm::normalize(towardPlayerView);

  glm::vec3 amountToMove = (direction); // * gameapi -> timeSeconds(false);

  auto elapsedTime = gameapi -> timeSeconds(false);
  amountToMove.x *= 10;
  amountToMove.y *= 10;
  amountToMove.z *= 10;
  std::cout << "amount to move: " << print(amountToMove) << std::endl;
  //gameapi -> setGameObjectPos(weapons.heldItem.value(), oldItemPos + amountToMove);
  gameapi -> applyForce(weapons.heldItem.value(), amountToMove);
}

void modifyPhysicsForHeldItem(Weapons& weapons){
  GameobjAttributes newAttr {
    .stringAttributes = {},
    .numAttributes = {},
    .vecAttr = { 
      .vec3 = { 
        { "physics_avelocity", glm::vec3(0.f, 0.f, 0.f) }, 
        { "physics_velocity", glm::vec3(0.f, 0.f, 0.f) },
        { "physics_angle", glm::vec3(0.f, 0.f, 0.f) }, 
        { "physics_linear", glm::vec3(1.f, 1.f, 1.f) }, 
        { "physics_gravity", glm::vec3(0.f, 0.f, 0.f) }, 
      }, 
      .vec4 = { } 
    },
  };
  gameapi -> setGameObjectAttr(weapons.heldItem.value(), newAttr);
}

bool isGunZoomed = false;
bool getIsGunZoomed(){
  return isGunZoomed;
}

void changeWeaponTargetId(Weapons& weapons, objid id){
  weapons.playerId = id;
  reloadTraitsValues(weapons);
  //changeGun(weapons, id, gameapi -> listSceneId(id), "pistol", 10);
}

CScriptBinding weaponBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Weapons* weapons = new Weapons;

    weapons -> playerId = std::nullopt;
    weapons -> isHoldingLeftMouse = false;
    weapons -> isHoldingRightMouse = false;
    weapons -> fireOnce = false;

    weapons -> lookVelocity = glm::vec2(0.f, 0.f);
    weapons -> movementVec = glm::vec3(0.f, 0.f, 0.f);

    weapons -> weaponValues.gunCore.weaponState = WeaponState {};
    weapons -> heldItem = std::nullopt;

  	return weapons;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Weapons* weapons = static_cast<Weapons*>(data);
    delete weapons;
  };
  binding.onMouseCallback = [](objid id, void* data, int button, int action, int mods) -> void {
    if (isPaused() || getGlobalState().disableGameInput){
      return;
    }
    Weapons* weapons = static_cast<Weapons*>(data);
    if (button == 0){
      if (action == 0){
        weapons -> isHoldingLeftMouse = false;
      }else if (action == 1){
        weapons -> isHoldingLeftMouse = true;
        weapons -> fireOnce = true;
      }
    }else if (button == 1){
      if (action == 0){
        weapons -> isHoldingRightMouse = false;
        isGunZoomed = false;
      }else if (action == 1){
        // select item
        weapons -> isHoldingRightMouse = true;
        isGunZoomed = true;
        if (weapons -> playerId.has_value()){
          auto hitpoints = doRaycast(glm::vec3(0.f, 0.f, -1.f), weapons -> playerId.value());
          if (hitpoints.size() > 0){
            auto cameraPos = gameapi -> getGameObjectPos(weapons -> playerId.value(), true);
            auto closestIndex = closestHitpoint(hitpoints, cameraPos);
            float distance = glm::length(cameraPos - hitpoints.at(closestIndex).point);
            if (distance <= weapons -> selectDistance){
              gameapi -> sendNotifyMessage("selected", hitpoints.at(closestIndex).id);
            }
          }
        }
      }
    }
  };
  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    if (isPaused() || getGlobalState().disableGameInput){
      return;
    }
    Weapons* weapons = static_cast<Weapons*>(data);
    if (!weapons -> playerId.has_value()){
      return;
    }
    if (key == 'E') { 
      if (action == 1){
        if (weapons -> heldItem.has_value()){
          modlog("weapons", "pickup released held item: " + std::to_string(weapons -> heldItem.value()));
          weapons -> heldItem = std::nullopt;
        }else{
        auto hitpoints = doRaycast(glm::vec3(0.f, 0.f, -1.f), weapons -> playerId.value());
        if (hitpoints.size() > 0){
            auto cameraPos = gameapi -> getGameObjectPos(weapons -> playerId.value(), true);
            auto closestHitpointIndex = closestHitpoint(hitpoints, cameraPos);
            auto hitpoint = hitpoints.at(closestHitpointIndex);
            float distance = glm::length(cameraPos - hitpoint.point);
            auto attrHandle = getAttrHandle(hitpoint.id);
            auto physicsEnabled = getStrAttr(attrHandle, "physics").value() == "enabled";
            auto physicsDynamic = getStrAttr(attrHandle, "physics_type").value() == "dynamic";
            auto physicsCollide = getStrAttr(attrHandle, "physics_collision").value() == "collide";
            auto canPickup = physicsEnabled && physicsDynamic && physicsCollide ;
            modlog("weapons", "pickup item: " + std::to_string(hitpoint.id) + " can pickup: " + print(canPickup) + " distance = " + std::to_string(distance));
            if (canPickup && distance < 5.f){
              weapons -> heldItem = hitpoint.id;
              modifyPhysicsForHeldItem(*weapons);
            }
          }
        }
      }
      return;
    }
  };

  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    Weapons* weapons = static_cast<Weapons*>(data);
    if (key == "change-gun"){
      auto changeGunMessage = anycast<ChangeGunMessage>(value); 
      modassert(changeGunMessage != NULL, "change-gun value invalid");
      changeGunAnimate(weapons -> weaponValues, changeGunMessage -> gun, changeGunMessage -> currentAmmo, gameapi -> listSceneId(id), weapons -> playerId.value());
    }else if (key == "ammo"){
      auto itemAcquiredMessage = anycast<ItemAcquiredMessage>(value);
      modassert(itemAcquiredMessage != NULL, "ammo message not an ItemAcquiredMessage");
      if (weapons -> playerId.has_value() && itemAcquiredMessage -> targetId == weapons -> playerId.value()){
        deliverAmmo(weapons -> weaponValues.gunCore, itemAcquiredMessage -> amount);
      }
    }else if (key == "save-gun"){
      saveGunTransform(weapons -> weaponValues);
    }else if (key == "velocity"){
      auto strValue = anycast<std::string>(value);   // would be nice to send the vec3 directly, but notifySend does not support
      modassert(strValue != NULL, "velocity value invalid");  
      weapons -> movementVec = parseVec(*strValue);
    }else if (key == "reload-config:weapon:traits"){
      Weapons* weapons = static_cast<Weapons*>(data);
      reloadTraitsValues(*weapons);
    }else if (key == "active-player-change"){
      Weapons* weapons = static_cast<Weapons*>(data);
      auto objIdValue = anycast<objid>(value); 
      modassert(objIdValue != NULL, "weapons - request change control value invalid");
      std::cout << "weapons want to change value: " << objIdValue << std::endl;
      changeWeaponTargetId(*weapons, *objIdValue);
    }
  };
  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void {
    if (isPaused() || getGlobalState().disableGameInput){
      return;
    }
    //std::cout << "mouse move: xPos = " << xPos << ", yPos = " << yPos << std::endl;
    Weapons* weapons = static_cast<Weapons*>(data);
    if (!weapons -> playerId.has_value()){
      return;
    }

    weapons -> lookVelocity = glm::vec2(xPos, yPos);
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    if (isPaused()){
      return;
    }
    Weapons* weapons = static_cast<Weapons*>(data);
    if (!weapons -> playerId.has_value()){
      return;
    }

    fireGunAndVisualize(weapons -> weaponValues.gunCore, weapons -> isHoldingLeftMouse, weapons -> fireOnce, weapons -> weaponValues.gunId, weapons -> weaponValues.muzzleId, weapons -> playerId.value());
    weapons -> fireOnce = false;
    swayGun(weapons -> weaponValues, weapons -> isHoldingRightMouse, weapons -> playerId.value(), weapons -> lookVelocity, weapons -> movementVec);
    handlePickedUpItem(*weapons);
    //std::cout << weaponsToString(*weapons) << std::endl;
  };

  binding.onObjectRemoved = [](int32_t _, void* data, int32_t idRemoved) -> void {
    // pretty sure i dont need this
    Weapons* weapons = static_cast<Weapons*>(data);
    if (weapons -> playerId.has_value() && weapons -> playerId.value() == idRemoved){
      weapons -> playerId = std::nullopt;
      removeGun(weapons -> weaponValues);
    }
  };

  return binding;
}

