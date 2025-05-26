#include "./weapon.h"

extern CustomApiBindings* gameapi;
bool entityInShootingMode(objid id);

bool showWeaponViewModel = true;

std::string weaponsToString(WeaponEntityState& weaponState){
  std::string str;
  str += std::string("isHoldingFire: ") + (weaponState.isHoldingFire ? "true" : "false") + "\n";
  str += std::string("isGunZoomed: ") + (weaponState.isGunZoomed ? "true" : "false") + "\n";
  return str;
}

// Should interpolate.  Looks better + prevent clipping bugs
// Might be interesting to incorporate things like mass and stuff
void handlePickedUpItem(WeaponEntityState& weaponState, objid playerId){
  if (!weaponState.heldItem.has_value()){
    return;
  }

  auto playerPos = gameapi -> getGameObjectPos(playerId, true, "[gamelogic] handlePickedUpItem player position");
  auto playerRotation = gameapi -> getGameObjectRotation(playerId, true);  // tempchecked  // maybe this should be the gun rotation instead, problem is the offsets on the gun
  glm::vec3 distanceFromPlayer = glm::vec3(0.f, 0.f, -5.f); 
  auto slightlyInFrontOfPlayer = gameapi -> moveRelativeVec(playerPos, playerRotation, distanceFromPlayer);

  // old object position
  auto oldItemPos = gameapi -> getGameObjectPos(weaponState.heldItem.value(), true, "[gamelogic] handlePickedUpItem held item location");
  auto towardPlayerView = slightlyInFrontOfPlayer - oldItemPos;
  auto distance = glm::length(towardPlayerView);
  auto direction = glm::normalize(towardPlayerView);

  glm::vec3 amountToMove = glm::vec3(direction.x * gameapi -> timeSeconds(false), direction.y * gameapi -> timeSeconds(false), direction.z * gameapi -> timeSeconds(false));

  amountToMove.x *= distance; 
  amountToMove.y *= distance;
  amountToMove.z *= distance;
  std::cout << "amount to move: " << print(amountToMove) << std::endl;
  //gameapi -> setGameObjectPos(weapons.heldItem.value(), oldItemPos + amountToMove);
  gameapi -> applyForce(weaponState.heldItem.value(), amountToMove);
}

std::optional<objid> raycastActivateableItem(objid playerId){
  auto hitpoints = doRaycastClosest(playerId, glm::vec3(0.f, 0.f, -1.f), std::nullopt);
  if (hitpoints.size() > 0){
    auto hitpoint = hitpoints.at(0);
    auto playerPos = gameapi -> getGameObjectPos(playerId, true, "[gamelogic] raycastActivateableItem get player position");
    auto distance = glm::distance(hitpoint.point, playerPos);
    if (distance < 2){
      modlog("active", "found item close");
      auto attrHandle = getAttrHandle(hitpoint.id);
      auto activateKey = getStrAttr(attrHandle, "activate");
      if (activateKey.has_value()){
        return hitpoint.id;
      }
    }
  }
  return std::nullopt;
}

void maybeChangeGun(WeaponEntityState& weaponState, std::string gun, objid inventory){
  if (hasGun(inventory, gun)){
    modlog("weapons change gun confirm", gun);
    changeGunAnimate(weaponState.weaponValues, gun);
  }else{
    modlog("weapons change gun - not in inventory", gun);
  }
}

std::optional<std::string*> getCurrentGunName(WeaponEntityState& weaponState){
  return getCurrentGunName(weaponState.weaponValues);
}
void deliverAmmoToCurrentGun(WeaponEntityState& weaponState, int amount, objid inventory){
  auto weaponName = getCurrentGunName(weaponState);
  if (weaponName.has_value()){
    deliverAmmo(inventory, *weaponName.value(), amount);
  }
}
AmmoInfo currentAmmoInfo(WeaponEntityState& weaponState, objid inventory){
  auto gunName = weaponState.weaponValues.gunCore.weaponCore -> weaponParams.name;
  auto currentAmmo = ammoForGun(inventory, gunName);
  auto totalAmmo = weaponState.weaponValues.gunCore.weaponCore  -> weaponParams.totalAmmo;
  return AmmoInfo {
    .currentAmmo = currentAmmo,
    .totalAmmo = totalAmmo,
  };
}

Weapons createWeapons(){
  Weapons weapons {
    .idToWeapon = {},
  };
  return weapons;
}

void addWeaponId(Weapons& weapons, objid id){
  modassert(weapons.idToWeapon.find(id) == weapons.idToWeapon.end(), "addWeaponId already in list");
  weapons.idToWeapon[id] = WeaponEntityState {
    .isHoldingFire = false,
    .fireOnce = false,
    .activate = false,
    .heldItem = std::nullopt,
    .holdToggle = HOLD_TOGGLE_NONE,
    .isGunZoomed = false,
  };
  weapons.idToWeapon.at(id).weaponValues.gunCore.weaponState = WeaponState {};
}

void removeWeaponId(Weapons& weapons, objid id){
  if (weapons.idToWeapon.find(id) != weapons.idToWeapon.end()){
    removeGun(getWeaponState(weapons, id).weaponValues);
  }
  weapons.idToWeapon.erase(id);
}

WeaponEntityState& getWeaponState(Weapons& weapons, objid id){
  modassert(weapons.idToWeapon.find(id) != weapons.idToWeapon.end(), "getWeaponState id not registered");
  return weapons.idToWeapon.at(id);
}


WeaponsUiUpdate onWeaponsFrameEntity(WeaponEntityState& weaponState, objid inventory, objid playerId, glm::vec2 lookVelocity, glm::vec3 playerVelocity, bool showFpsGun, bool showThirdPersonGun, std::function<objid(objid)> getWeaponParentId, ThirdPersonWeapon thirdPersonWeapon, FiringTransform& fireTransform){
  ensureGunInstance(weaponState.weaponValues, playerId, showFpsGun, showThirdPersonGun, getWeaponParentId, thirdPersonWeapon);

  if (weaponState.activate){
    auto activateableItem = raycastActivateableItem(playerId);
    if (activateableItem.has_value()){
      auto attrHandle = getAttrHandle(activateableItem.value());
      auto activateKey = getStrAttr(attrHandle, "activate");
      auto activateValue = getStrAttr(attrHandle, "activate-value");
      if (activateKey.has_value()){
        auto pos = gameapi -> getGameObjectPos(activateableItem.value(), true, "[gamelogic] onWeaponsFrameEntity - activatable item sound location");
        playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, pos);

        MessageWithId activateMessage {
          .id = activateableItem.value(),
          .value = activateValue,
        };
        gameapi -> sendNotifyMessage(activateKey.value(), activateMessage);
      }
    }    
  }
  weaponState.activate = false;

  if(weaponState.holdToggle == HOLD_TOGGLE_PICKUP || weaponState.holdToggle == HOLD_TOGGLE_RELEASE){
    if (weaponState.heldItem.has_value()){
      modlog("weapons", "pickup released held item: " + std::to_string(weaponState.heldItem.value()));
      if (weaponState.heldItem.has_value()){
        setGameObjectPhysicsOptions(  // this should instead restore the properties when it was picked up
          weaponState.heldItem.value(), 
          glm::vec3(0.f, 0.f, 0.f), 
          glm::vec3(0.f, 0.f, 0.f), 
          glm::vec3(0.f, 0.f, 0.f), 
          glm::vec3(1.f, 1.f, 1.f), 
          glm::vec3(0.f, -9.81f, 0.f)
        );
      }
      weaponState.heldItem = std::nullopt;
    }else{
      auto mainobjPos = gameapi -> getGameObjectPos(playerId, true, "[gamelogic] onWeaponsFrameEntity - entity location");
      auto mainobjRotation = gameapi -> getGameObjectRotation(playerId, true);  // tempchecked

      auto hitpoints = doRaycast(glm::vec3(0.f, 0.f, -1.f), mainobjPos, mainobjRotation);
      if (hitpoints.size() > 0){
        auto cameraPos = gameapi -> getGameObjectPos(playerId, true, "[gamelogic] onWeaponsFrameEntity - entity location 2");
        auto closestHitpointIndex = closestHitpoint(hitpoints, cameraPos, std::nullopt).value();
        auto hitpoint = hitpoints.at(closestHitpointIndex);
        float distance = glm::length(cameraPos - hitpoint.point);
        auto attrHandle = getAttrHandle(hitpoint.id);
        auto physicsEnabled = getBoolAttr(attrHandle, "physics").value();
        auto physicsDynamic = getBoolAttr(attrHandle, "physics_type").value() == false;
        auto physicsCollide = getBoolAttr(attrHandle, "physics_collision").value() == true;
        auto canPickup = physicsEnabled && physicsDynamic && physicsCollide ;
        modlog("weapons", "pickup item: " + std::to_string(hitpoint.id) + " can pickup: " + print(canPickup) + " distance = " + std::to_string(distance));
        if (canPickup && distance < 5.f){
          weaponState.heldItem = hitpoint.id;
          setGameObjectPhysicsOptions(
            weaponState.heldItem.value(), 
            glm::vec3(0.f, 0.f, 0.f), 
            glm::vec3(0.f, 0.f, 0.f), 
            glm::vec3(0.f, 0.f, 0.f), 
            glm::vec3(1.f, 1.f, 1.f), 
            glm::vec3(0.f, 0.f, 0.f)
          );
        }
      }
    }
  }
  weaponState.holdToggle = HOLD_TOGGLE_NONE;

  auto isInShootingMode = entityInShootingMode(playerId);
  auto gunFireInfo = fireGunAndVisualize(weaponState.weaponValues.gunCore, weaponState.isHoldingFire, weaponState.fireOnce, weaponState.weaponValues.gunId, weaponState.weaponValues.muzzleId, playerId, inventory, fireTransform, isInShootingMode);

  weaponState.fireOnce = false;
  swayGun(weaponState.weaponValues, weaponState.isGunZoomed, playerId, lookVelocity, playerVelocity);
  handlePickedUpItem(weaponState, playerId);
  auto showActivateUi = raycastActivateableItem(playerId).has_value();

  if (!getCurrentGunName(weaponState).has_value()){
    return WeaponsUiUpdate { 
      .ammoInfo = std::nullopt, 
      .showActivateUi = showActivateUi,
      .bloomAmount = gunFireInfo.bloomAmount,
      .currentGunName = getCurrentGunName(weaponState),
    };
  }
  return WeaponsUiUpdate {
    .ammoInfo = currentAmmoInfo(weaponState, inventory),
    .showActivateUi = showActivateUi,
    .bloomAmount = gunFireInfo.bloomAmount,
    .currentGunName = getCurrentGunName(weaponState),
  };
}


WeaponsUiUpdate onWeaponsFrame(Weapons& weapons, objid playerId, glm::vec2 lookVelocity, glm::vec3 playerVelocity, std::function<WeaponEntityData(objid)> getWeaponEntityData, std::function<objid(objid)> getWeaponParentId, ThirdPersonWeapon thirdPersonWeapon) {
  WeaponsUiUpdate weaponsUiUpdate{
    .ammoInfo = std::nullopt,
    .showActivateUi = false,
  };
  for (auto &[id, weaponEntityState] : weapons.idToWeapon){
    auto weaponEntityData = getWeaponEntityData(id);
    bool activePlayer = id == playerId;

    auto uiUpdate = onWeaponsFrameEntity(weaponEntityState, weaponEntityData.inventory, id, weaponEntityData.lookVelocity, weaponEntityData.velocity, showWeaponViewModel && activePlayer && !weaponEntityData.thirdPersonMode, weaponEntityData.thirdPersonMode, getWeaponParentId, thirdPersonWeapon, weaponEntityData.fireTransform);
    if (activePlayer){
      weaponsUiUpdate = uiUpdate;
    }
  }
  return weaponsUiUpdate;
}

const float zoomAmount = 4.f;
WeaponsMouseUpdate onWeaponsMouseCallback(WeaponEntityState& weaponsState, int button, int action, objid playerId, float selectDistance){
  std::optional<float> zoomUpdateAmount;
  std::optional<bool> zoomUpdate;
  if (isFireButton(button)){
    if (action == 0){
      weaponsState.isHoldingFire = false;
    }else if (action == 1){
      weaponsState.isHoldingFire = true;
      weaponsState.fireOnce = true;
    }
  }else if (isAimButton(button)){
    if (action == 0){
      weaponsState.isGunZoomed = false;
      zoomUpdate = false;
      zoomUpdateAmount = 1.f;
    }else if (action == 1){
      // select item
      weaponsState.isGunZoomed = true;
      zoomUpdate = true;
      zoomUpdateAmount = 1.f / zoomAmount;
    }
  }
  return WeaponsMouseUpdate {
    .zoomAmount = zoomUpdateAmount,
    .zoomUpdate = zoomUpdate,
  };
}

void onWeaponsKeyCallback(WeaponEntityState& weaponsState, int key, int action, objid playerId){
  if (isInteractKey(key)) { 
    if (action == 1){
      weaponsState.activate = true;
      weaponsState.holdToggle = HOLD_TOGGLE_PICKUP;
    }else if (action == 0){
      weaponsState.holdToggle = HOLD_TOGGLE_RELEASE;
    }
  }
}

void setShowWeaponModel(bool showModel){
  showWeaponViewModel = showModel;
}

void fireGun(Weapons& weapons, objid playerId){
  WeaponEntityState& weaponState = getWeaponState(weapons, playerId);
  weaponState.fireOnce = true;
}