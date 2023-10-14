#include "./weapon.h"

extern CustomApiBindings* gameapi;

struct Weapons {
  std::vector<MaterialToParticle> materials;

  objid playerId;
  objid sceneId;
  bool isHoldingLeftMouse;
  bool isHoldingRightMouse;
  float selectDistance;

  WeaponValues weaponValues;

  glm::vec2 lookVelocity;
  float movementVelocity;
  glm::vec3 movementVec;

  std::optional<objid> raycastLine;
  std::optional<objid> heldItem;
};

void saveGunTransform(Weapons& weapons){
  debugAssertForNow(false, "bad code - cannot get raw position / etc since ironsights mean this needs to subtract by initial offset");

  if (weapons.weaponValues.weaponInstance.has_value()){
    auto gunId = weapons.weaponValues.weaponInstance.value().gunId;
    auto gun = weapons.weaponValues.weaponParams.name;
    auto attr = gameapi -> getGameObjectAttr(gunId);
    auto position = attr.vecAttr.vec3.at("position");  
    auto scale = attr.vecAttr.vec3.at("scale");
    auto rotation = attr.vecAttr.vec4.at("rotation");

    modlog("weapons", "save gun, name = " + weapons.weaponValues.weaponParams.name + ",  pos = " + print(position) + ", scale = " + print(scale) + ", rotation = " + print(rotation));

    auto updateQuery = gameapi -> compileSqlQuery(
      std::string("update guns set ") +
        "xoffset-pos = " + serializeFloat(position.x) + ", " +
        "yoffset-pos = " + serializeFloat(position.y) + ", " +
        "zoffset-pos = " + serializeFloat(position.z) + ", " + 
        "xrot = " + serializeFloat(rotation.x) + ", " +
        "yrot = " + serializeFloat(rotation.y) + ", " +
        "zrot = " + serializeFloat(rotation.z) + 
        " where name = " + gun,
        {}
    );
    bool validSql = false;
    auto result = gameapi -> executeSqlQuery(updateQuery, &validSql);
    modassert(validSql, "error executing sql query");
  }
}


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


float calculateBloomAmount(Weapons& weapons){
  auto slerpAmount = (1 - calcRecoilSlerpAmount(weapons.weaponValues, weapons.weaponValues.weaponParams.bloomLength, false)); 
  modassert(slerpAmount <= 1, "slerp amount must be less than 1, got: " + std::to_string(slerpAmount));
  return glm::max(weapons.weaponValues.weaponParams.minBloom, (weapons.weaponValues.weaponParams.totalBloom - weapons.weaponValues.weaponParams.minBloom) * slerpAmount + weapons.weaponValues.weaponParams.minBloom);
}


// Should interpolate.  Looks better + prevent clipping bugs
// Might be interesting to incorporate things like mass and stuff
void handlePickedUpItem(Weapons& weapons){
  if (!weapons.heldItem.has_value()){
    return;
  }

  auto playerPos = gameapi -> getGameObjectPos(weapons.playerId, true);
  auto playerRotation = gameapi -> getGameObjectRotation(weapons.playerId, true);  // maybe this should be the gun rotation instead, problem is the offsets on the gun
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

void changeWeaponTargetId(Weapons& weapons, objid id){
  weapons.playerId = id;
  reloadTraitsValues(weapons);
  //changeGun(weapons, id, gameapi -> listSceneId(id), "pistol", 10);
}

CScriptBinding weaponBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Weapons* weapons = new Weapons;
    weapons -> materials = loadMaterials(sceneId);

    weapons -> playerId = gameapi -> getGameObjectByName(">maincamera", sceneId, false).value();
    weapons -> sceneId = sceneId;
    weapons -> isHoldingLeftMouse = false;
    weapons -> isHoldingRightMouse = false;

    weapons -> lookVelocity = glm::vec2(0.f, 0.f);
    weapons -> movementVelocity = 0.f;
    weapons -> movementVec = glm::vec3(0.f, 0.f, 0.f);
    weapons -> raycastLine = std::nullopt;

    weapons -> weaponValues.weaponState = WeaponState {};

    weapons -> heldItem = std::nullopt;

    changeWeaponTargetId(*weapons, weapons -> playerId);
  	return weapons;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Weapons* weapons = static_cast<Weapons*>(data);
    delete weapons;
  };
  binding.onMouseCallback = [](objid id, void* data, int button, int action, int mods) -> void {
    if (isPaused()){
      return;
    }
    Weapons* weapons = static_cast<Weapons*>(data);
    if (button == 0){
      if (action == 0){
        weapons -> isHoldingLeftMouse = false;
      }else if (action == 1){
        weapons -> isHoldingLeftMouse = true;
        tryFireGun(weapons -> weaponValues, gameapi -> listSceneId(id), calculateBloomAmount(*weapons), weapons -> playerId, weapons -> materials);
      }
    }else if (button == 1){
      if (action == 0){
        weapons -> isHoldingRightMouse = false;
      }else if (action == 1){
        // select item
        weapons -> isHoldingRightMouse = true;
        auto hitpoints = doRaycast(glm::vec3(0.f, 0.f, -1.f), weapons -> playerId);
        if (hitpoints.size() > 0){
          auto cameraPos = gameapi -> getGameObjectPos(weapons -> playerId, true);
          auto closestIndex = closestHitpoint(hitpoints, cameraPos);
          float distance = glm::length(cameraPos - hitpoints.at(closestIndex).point);
          if (distance <= weapons -> selectDistance){
            gameapi -> sendNotifyMessage("selected", hitpoints.at(closestIndex).id);
          }
        }
      }
    }
  };
  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    Weapons* weapons = static_cast<Weapons*>(data);
    if (key == 'E') { 
      if (action == 1){
        if (weapons -> heldItem.has_value()){
          modlog("weapons", "pickup released held item: " + std::to_string(weapons -> heldItem.value()));
          weapons -> heldItem = std::nullopt;
        }else{
        auto hitpoints = doRaycast(glm::vec3(0.f, 0.f, -1.f), weapons -> playerId);
        if (hitpoints.size() > 0){
            auto cameraPos = gameapi -> getGameObjectPos(weapons -> playerId, true);
            auto closestHitpointIndex = closestHitpoint(hitpoints, cameraPos);
            auto hitpoint = hitpoints.at(closestHitpointIndex);
            float distance = glm::length(cameraPos - hitpoint.point);
            auto attr = gameapi -> getGameObjectAttr(hitpoint.id);
            auto physicsEnabled = getStrAttr(attr, "physics").value() == "enabled";
            auto physicsDynamic = getStrAttr(attr, "physics_type").value() == "dynamic";
            auto physicsCollide = getStrAttr(attr, "physics_collision").value() == "collide";
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
      auto value = changeGunMessage -> gun;
      auto currentAmmo = changeGunMessage -> currentAmmo;
      if (value != weapons -> weaponValues.weaponParams.name){
        gameapi -> sendNotifyMessage("set-gun-ammo", SetAmmoMessage {
          .currentAmmo = weapons -> weaponValues.weaponState.currentAmmo,
          .gun = weapons -> weaponValues.weaponParams.name,
        });

        weapons -> weaponValues.weaponState.gunState = GUN_LOWERING;
        gameapi -> schedule(id, 1000, weapons, [id, value, currentAmmo](void* weaponData) -> void {
          Weapons* weaponValue = static_cast<Weapons*>(weaponData);
          changeGun(weaponValue -> weaponValues, value, currentAmmo, gameapi -> listSceneId(id), weaponValue -> playerId);
        });
      }
    }else if (key == "ammo"){
      auto intValue = anycast<int>(value);
      modassert(intValue != NULL, "ammo message not an int");
      deliverAmmo(weapons -> weaponValues, *intValue);
    }else if (key == "save-gun"){
      saveGunTransform(*weapons);
    }else if (key == "velocity"){
      auto strValue = anycast<std::string>(value);   // would be nice to send the vec3 directly, but notifySend does not support
      modassert(strValue != NULL, "velocity value invalid");  
      weapons -> movementVec = parseVec(*strValue);
      weapons -> movementVelocity = glm::length(weapons -> movementVec);
      //std::cout << "speed is: " << weapons -> movementVelocity << std::endl;
    }else if (key == "reload-config:weapon:traits"){
      Weapons* weapons = static_cast<Weapons*>(data);
      reloadTraitsValues(*weapons);
    }else if (key == "request:change-control"){
      Weapons* weapons = static_cast<Weapons*>(data);
      auto objIdValue = anycast<objid>(value); 
      modassert(objIdValue != NULL, "weapons - request change control value invalid");
      std::cout << "weapons want to change value: " << objIdValue << std::endl;
      changeWeaponTargetId(*weapons, *objIdValue);
    }
  };
  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void { 
    if (isPaused()){
      return;
    }
    //std::cout << "mouse move: xPos = " << xPos << ", yPos = " << yPos << std::endl;
    Weapons* weapons = static_cast<Weapons*>(data);
    weapons -> lookVelocity = glm::vec2(xPos, yPos);
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    if (isPaused()){
      return;
    }
    Weapons* weapons = static_cast<Weapons*>(data);
    auto bloomAmount = calculateBloomAmount(*weapons);
    drawBloom(weapons -> playerId, id, -1.f, bloomAmount); // 0.002f is just a min amount for visualization, not actual bloom
    if (weapons -> weaponValues.weaponParams.canHold && weapons -> isHoldingLeftMouse){
      tryFireGun(weapons -> weaponValues, gameapi -> listSceneId(id), bloomAmount, weapons -> playerId, weapons -> materials);
    }
    swayGun(weapons -> weaponValues, weapons -> isHoldingRightMouse, weapons -> playerId, weapons -> lookVelocity, weapons -> movementVec);
    handlePickedUpItem(*weapons);
    //std::cout << weaponsToString(*weapons) << std::endl;
  };
  return binding;
}

