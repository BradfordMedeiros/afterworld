#include "./weapon.h"

extern CustomApiBindings* gameapi;

std::string weaponsToString(Weapons& weapons){
  std::string str;
  str += std::string("isHoldingFire: ") + (weapons.state.isHoldingFire ? "true" : "false") + "\n";
  str += std::string("isGunZoomed: ") + (weapons.state.isGunZoomed ? "true" : "false") + "\n";
  return str;
}

// Should interpolate.  Looks better + prevent clipping bugs
// Might be interesting to incorporate things like mass and stuff
void handlePickedUpItem(Weapons& weapons, objid playerId){
  if (!weapons.state.heldItem.has_value()){
    return;
  }

  auto playerPos = gameapi -> getGameObjectPos(playerId, true);
  auto playerRotation = gameapi -> getGameObjectRotation(playerId, true);  // maybe this should be the gun rotation instead, problem is the offsets on the gun
  glm::vec3 distanceFromPlayer = glm::vec3(0.f, 0.f, -5.f); 
  auto slightlyInFrontOfPlayer = gameapi -> moveRelativeVec(playerPos, playerRotation, distanceFromPlayer);

  // old object position
  auto oldItemPos = gameapi -> getGameObjectPos(weapons.state.heldItem.value(), true);
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
  gameapi -> applyForce(weapons.state.heldItem.value(), amountToMove);
}

std::optional<objid> raycastActivateableItem(Weapons& weapons, objid playerId){
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
        return hitpoint.id;
      }
    }
  }
  return std::nullopt;
}

void maybeChangeGun(Weapons& weapons, std::string gun, std::string& inventory, objid playerId){
  if (hasGun(inventory, gun)){
    modlog("weapons change gun confirm", gun);
    changeGunAnimate(weapons.state.weaponValues, gun, ammoForGun(inventory, gun), gameapi -> listSceneId(playerId), playerId);
  }else{
    modlog("weapons change gun - not in inventory", gun);
  }
}

std::optional<std::string*> getCurrentGunName(Weapons& weapons){
  if (!weapons.state.weaponValues.gunCore.weaponCore){
    return std::nullopt;
  }
  return &weapons.state.weaponValues.gunCore.weaponCore -> weaponParams.name;
}
void deliverAmmoToCurrentGun(Weapons& weapons, int amount, std::string& inventory){
  auto weaponName = getCurrentGunName(weapons);
  if (weaponName.has_value()){
    deliverAmmo(inventory, *weaponName.value(), amount);
  }
}
AmmoInfo currentAmmoInfo(Weapons& weapons, std::string& inventory){
  auto gunName = weapons.state.weaponValues.gunCore.weaponCore -> weaponParams.name;
  auto currentAmmo = ammoForGun(inventory, gunName);
  auto totalAmmo = weapons.state.weaponValues.gunCore.weaponCore  -> weaponParams.totalAmmo;
  return AmmoInfo {
    .currentAmmo = currentAmmo,
    .totalAmmo = totalAmmo,
  };
}

Weapons createWeapons(){
  Weapons weapons {
    .state = WeaponEntityState {
      .isHoldingFire = false,
      .fireOnce = false,
      .heldItem = std::nullopt,
      .isGunZoomed = false,
    },
  };
  weapons.state.weaponValues.gunCore.weaponState = WeaponState {};
  return weapons;
}

WeaponsUiUpdate onWeaponsFrame(Weapons& weapons, std::string& inventory, objid playerId, glm::vec2 lookVelocity){
  bool didFire = fireGunAndVisualize(weapons.state.weaponValues.gunCore, weapons.state.isHoldingFire, weapons.state.fireOnce, weapons.state.weaponValues.gunId, weapons.state.weaponValues.muzzleId, playerId, inventory);
  weapons.state.fireOnce = false;
  swayGun(weapons.state.weaponValues, weapons.state.isGunZoomed, playerId, lookVelocity, getPlayerVelocity());
  handlePickedUpItem(weapons, playerId);
  auto showActivateUi = raycastActivateableItem(weapons, playerId).has_value();

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
  removeGun(weapons.state.weaponValues);
}

const float zoomAmount = 4.f;
WeaponsMouseUpdate onWeaponsMouseCallback(Weapons& weapons, int button, int action, objid playerId, float selectDistance){
  std::optional<float> zoomUpdateAmount;
  std::optional<objid> selectItem;
  if (isFireButton(button)){
    if (action == 0){
      weapons.state.isHoldingFire = false;
    }else if (action == 1){
      weapons.state.isHoldingFire = true;
      weapons.state.fireOnce = true;
    }
  }else if (isAimButton(button)){
    if (action == 0){
      weapons.state.isGunZoomed = false;
      zoomUpdateAmount = 1.f;
    }else if (action == 1){
      // select item
      weapons.state.isGunZoomed = true;
      zoomUpdateAmount = 1.f / zoomAmount;
      auto hitpoints = doRaycast(glm::vec3(0.f, 0.f, -1.f), playerId);
      if (hitpoints.size() > 0){
        auto cameraPos = gameapi -> getGameObjectPos(playerId, true);
        auto closestIndex = closestHitpoint(hitpoints, cameraPos);
        float distance = glm::length(cameraPos - hitpoints.at(closestIndex).point);
        if (distance <= selectDistance){
          selectItem = hitpoints.at(closestIndex).id;
        }
      }
    }
  }
  return WeaponsMouseUpdate {
    .zoomAmount = zoomUpdateAmount,
    .selectItem = selectItem,
  };
}

void onWeaponsKeyCallback(Weapons& weapons, int key, int action, objid playerId){
  if (isInteractKey(key)) { 
    if (action == 1){
      auto activateableItem = raycastActivateableItem(weapons, playerId);
      if (activateableItem.has_value()){
        auto attrHandle = getAttrHandle(activateableItem.value());
        auto activateKey = getStrAttr(attrHandle, "activate");
        if (activateKey.has_value()){
          auto pos = gameapi -> getGameObjectPos(activateableItem.value(), true);
          playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, pos);
          gameapi -> sendNotifyMessage(activateKey.value(), "default");
        }
      }
      if (weapons.state.heldItem.has_value()){
        modlog("weapons", "pickup released held item: " + std::to_string(weapons.state.heldItem.value()));
        weapons.state.heldItem = std::nullopt;
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
            weapons.state.heldItem = hitpoint.id;
            setGameObjectPhysicsOptions(
              weapons.state.heldItem.value(), 
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

