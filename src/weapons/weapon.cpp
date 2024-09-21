#include "./weapon.h"

extern CustomApiBindings* gameapi;
void setTotalZoom(float multiplier);
void handleSelectItem(objid id);

std::string weaponsToString(Weapons& weapons){
  std::string str;
  str += std::string("isHoldingLeftMouse: ") + (weapons.controls.isHoldingLeftMouse ? "true" : "false") + "\n";
  str += std::string("isHoldingRightMouseo: ") + (weapons.controls.isHoldingRightMouse ? "true" : "false") + "\n";
  return str;
}

// Should interpolate.  Looks better + prevent clipping bugs
// Might be interesting to incorporate things like mass and stuff
void handlePickedUpItem(Weapons& weapons, objid playerId){
  if (!weapons.heldItem.has_value()){
    return;
  }

  auto playerPos = gameapi -> getGameObjectPos(playerId, true);
  auto playerRotation = gameapi -> getGameObjectRotation(playerId, true);  // maybe this should be the gun rotation instead, problem is the offsets on the gun
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

// returns if should show active
bool handleActivateItem(Weapons& weapons, objid playerId){
  auto hitpoints = doRaycastClosest(glm::vec3(0.f, 0.f, -1.f), playerId);
  if (hitpoints.size() > 0){
    auto hitpoint = hitpoints.at(0);
    auto playerPos = gameapi -> getGameObjectPos(playerId, true);
    auto distance = glm::distance(hitpoint.point, playerPos);
    if (distance < 2){
      modlog("active", "found item close");
      auto attrHandle = getAttrHandle(hitpoint.id);
      auto activateKey = getStrAttr(attrHandle, "activate");
      if (activateKey.has_value()){
        weapons.activateableItem = hitpoint.id;
        return true;        
      }
    }
  }
  weapons.activateableItem = std::nullopt;
  return false;
}

void maybeChangeGun(Weapons& weapons, std::string gun, std::string& inventory, objid playerId){
  if (hasGun(inventory, gun)){
    modlog("weapons change gun confirm", gun);
    changeGunAnimate(weapons.weaponValues, gun, ammoForGun(inventory, gun), gameapi -> listSceneId(playerId), playerId);
  }else{
    modlog("weapons change gun - not in inventory", gun);
  }
}

std::optional<std::string*> getCurrentGunName(Weapons& weapons){
  if (!weapons.weaponValues.gunCore.weaponCore){
    return std::nullopt;
  }
  return &weapons.weaponValues.gunCore.weaponCore -> weaponParams.name;
}
void deliverAmmoToCurrentGun(Weapons& weapons, objid targetId, int amount, std::string& inventory, objid playerId){
  if (targetId == playerId){
    auto weaponName = getCurrentGunName(weapons);
    if (weaponName.has_value()){
      deliverAmmo(inventory, *weaponName.value(), amount);
    }
  }
}

AmmoInfo currentAmmoInfo(Weapons& weapons, std::string& inventory){
  auto gunName = weapons.weaponValues.gunCore.weaponCore -> weaponParams.name;
  auto currentAmmo = ammoForGun(inventory, gunName);
  auto totalAmmo = weapons.weaponValues.gunCore.weaponCore  -> weaponParams.totalAmmo;

  return AmmoInfo {
    .currentAmmo = currentAmmo,
    .totalAmmo = totalAmmo,
  };
}

Weapons createWeapons(){
  Weapons weapons {
    .controls = WeaponControls{
      .isHoldingLeftMouse = false,
      .isHoldingRightMouse = false,
      .fireOnce = false,
    },
    .heldItem = std::nullopt,
    .isGunZoomed = false,
    .activateableItem = std::nullopt,
  };
  weapons.weaponValues.gunCore.weaponState = WeaponState {};
  return weapons;
}

WeaponsUiUpdate onWeaponsFrame(Weapons& weapons, std::string& inventory, objid playerId, glm::vec2 lookVelocity){
  if (isPaused()){
    return WeaponsUiUpdate { 
      .ammoInfo = std::nullopt,
      .showActivateUi = false,
    };
  }
  bool didFire = fireGunAndVisualize(weapons.weaponValues.gunCore, weapons.controls.isHoldingLeftMouse, weapons.controls.fireOnce, weapons.weaponValues.gunId, weapons.weaponValues.muzzleId, playerId, inventory);
  weapons.controls.fireOnce = false;
  swayGun(weapons.weaponValues, weapons.controls.isHoldingRightMouse, playerId, lookVelocity, getPlayerVelocity());
  handlePickedUpItem(weapons, playerId);
  auto showActivateUi = handleActivateItem(weapons, playerId);

  if (!getCurrentGunName(weapons).has_value()){
    return WeaponsUiUpdate { 
      .ammoInfo = std::nullopt, 
      .showActivateUi = showActivateUi,
    };
  }
  return WeaponsUiUpdate {
    .ammoInfo = currentAmmoInfo(weapons, inventory),
    .showActivateUi = showActivateUi,
  };
}

void removeActiveGun(Weapons& weapons){
  removeGun(weapons.weaponValues);
}

const float zoomAmount = 4.f;
void onWeaponsMouseCallback(Weapons& weapons, int button, int action, objid playerId, float selectDistance){
  if (isPaused() || getGlobalState().disableGameInput){
    return;
  }
  if (isFireButton(button)){
    if (action == 0){
      weapons.controls.isHoldingLeftMouse = false;
    }else if (action == 1){
      weapons.controls.isHoldingLeftMouse = true;
      weapons.controls.fireOnce = true;
    }
  }else if (isAimButton(button)){
    if (action == 0){
      weapons.controls.isHoldingRightMouse = false;
      weapons.isGunZoomed = false;
      setTotalZoom(weapons.isGunZoomed ? (1.f / zoomAmount) : 1.f);
    }else if (action == 1){
      // select item
      weapons.controls.isHoldingRightMouse = true;
      weapons.isGunZoomed = true;
      setTotalZoom(weapons.isGunZoomed ? (1.f / zoomAmount) : 1.f);
      auto hitpoints = doRaycast(glm::vec3(0.f, 0.f, -1.f), playerId);
      if (hitpoints.size() > 0){
        auto cameraPos = gameapi -> getGameObjectPos(playerId, true);
        auto closestIndex = closestHitpoint(hitpoints, cameraPos);
        float distance = glm::length(cameraPos - hitpoints.at(closestIndex).point);
        if (distance <= selectDistance){
          handleSelectItem(hitpoints.at(closestIndex).id);
        }
      }
    }
  }
}

void onWeaponsKeyCallback(Weapons& weapons, int key, int action, objid playerId){
  if (isPaused() || getGlobalState().disableGameInput){
    return;
  }
  if (isInteractKey(key)) { 
    if (action == 1){
      if (weapons.activateableItem.has_value()){
        auto attrHandle = getAttrHandle(weapons.activateableItem.value());
        auto activateKey = getStrAttr(attrHandle, "activate");
        if (activateKey.has_value()){
          auto pos = gameapi -> getGameObjectPos(weapons.activateableItem.value(), true);
          playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, pos);
          gameapi -> sendNotifyMessage(activateKey.value(), "default");
        }
      }
      if (weapons.heldItem.has_value()){
        modlog("weapons", "pickup released held item: " + std::to_string(weapons.heldItem.value()));
        weapons.heldItem = std::nullopt;
      }else{
      auto hitpoints = doRaycast(glm::vec3(0.f, 0.f, -1.f), playerId);
      if (hitpoints.size() > 0){
          auto cameraPos = gameapi -> getGameObjectPos(playerId, true);
          auto closestHitpointIndex = closestHitpoint(hitpoints, cameraPos);
          auto hitpoint = hitpoints.at(closestHitpointIndex);
          float distance = glm::length(cameraPos - hitpoint.point);
          auto attrHandle = getAttrHandle(hitpoint.id);
          auto physicsEnabled = getBoolAttr(attrHandle, "physics").value();
          auto physicsDynamic = getBoolAttr(attrHandle, "physics_type").value() == false;
          auto physicsCollide = getBoolAttr(attrHandle, "physics_collision").value() == true;
          auto canPickup = physicsEnabled && physicsDynamic && physicsCollide ;
          modlog("weapons", "pickup item: " + std::to_string(hitpoint.id) + " can pickup: " + print(canPickup) + " distance = " + std::to_string(distance));
          if (canPickup && distance < 5.f){
            weapons.heldItem = hitpoint.id;
            setGameObjectPhysicsOptions(
              weapons.heldItem.value(), 
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
    return;
  }
}

