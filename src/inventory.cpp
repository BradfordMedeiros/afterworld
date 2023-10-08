#include "./inventory.h"

extern CustomApiBindings* gameapi;

int ensureItemExists(std::string name){
  auto query = gameapi -> compileSqlQuery(std::string("select item, count from inventory where item = ") +  name, {});
	bool validSql = false;
	auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  modassert(result.size() == 0 || result.size() == 1, "more than 1 entry for item exists");

  if (result.size() == 0){
   	auto insertQuery = gameapi -> compileSqlQuery(
  	  std::string("insert into inventory (item, count) values (") +  name + ", 0)",
      {}
  	);

		bool validSql = false;
	  gameapi -> executeSqlQuery(insertQuery, &validSql);
  	modassert(validSql, "error executing sql query");	
  }

  auto count = result.size() == 0 ? 0 : std::atoi(result.at(0).at(1).c_str());
	return count;
}

void updateItemCount(std::string name, int count){
  auto updateQuery = gameapi -> compileSqlQuery(
    std::string("update inventory set count = " + std::to_string(count) + " where item = " + name),
    {}
  );
	bool validSql = false;
	auto result = gameapi -> executeSqlQuery(updateQuery, &validSql);
  modassert(validSql, "error executing sql query");
}

bool hasGun(std::string& gun){
  auto query = gameapi -> compileSqlQuery("select count from inventory where item = ?", { gun });
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  return result.size() > 0;
}
int ammoForGun(std::string& gun){
  auto query = gameapi -> compileSqlQuery("select count from inventory where item = ?", { gun + "-ammo" });
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  if (result.size() == 0){
    return 0;
  }
  return std::atoi(result.at(0).at(0).c_str());
}

bool isPlayer(objid id){
  auto playerAttr = getSingleAttr(id, "player");
  return playerAttr.has_value() && playerAttr.value() == "true";
}
bool isPickup(objid id){
  auto playerAttr = getSingleAttr(id, "pickup");
  return playerAttr.has_value();
}

void tryPickupItem(objid gameObjId){
  auto objAttr =  gameapi -> getGameObjectAttr(gameObjId);
  auto pickup = getStrAttr(objAttr, "pickup");
  if (pickup.has_value()){
    auto pickupTrigger = getStrAttr(objAttr, "pickup-trigger");
    auto pickupQuantity = getFloatAttr(objAttr, "pickup-amount");
    auto pickupType = getStrAttr(objAttr, "pickup-type");
    auto quantityAmount = pickupQuantity.has_value() ? pickupQuantity.value() : 1.f;

    auto oldItemCount = ensureItemExists(pickup.value());
    auto newItemCount = (pickupType.has_value() && pickupType.value() == "replace") ? quantityAmount : (oldItemCount + quantityAmount);
    updateItemCount(pickup.value(), newItemCount);

    gameapi -> removeObjectById(gameObjId);

    if (pickupTrigger.has_value()){
      gameapi -> sendNotifyMessage(pickupTrigger.value(), static_cast<int>(newItemCount));
    }
  }
}



CScriptBinding inventoryBinding(CustomApiBindings& api, const char* name){
	 auto binding = createCScriptBinding(name, api);
   binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
   	if (key == "selected"){  // maybe this logic should be somewhere else and not be in dialog
      auto gameObjId = anycast<objid>(value); 
      modassert(gameObjId != NULL, "inventory selected value invalid");
      if (!gameapi -> getGameObjNameForId(*gameObjId).has_value()){
        return;
      }
      tryPickupItem(*gameObjId);
    }

    if (key == "request-change-gun"){
      auto strValue = anycast<std::string>(value); 
      modassert(strValue != NULL, "selected value invalid");
      if (hasGun(*strValue)){
        ChangeGunMessage changeGun {
          .currentAmmo = ammoForGun(*strValue),
          .gun = *strValue,
        };
        gameapi -> sendNotifyMessage("change-gun", changeGun);
      }
    }
    if (key == "set-gun-ammo"){
      auto setAmmoMessage = anycast<SetAmmoMessage>(value); 
      modassert(setAmmoMessage != NULL, "setAmmoMessage invalid");
      updateItemCount(setAmmoMessage -> gun + "-ammo", setAmmoMessage -> currentAmmo);
    }
  };

  binding.onCollisionEnter = [](objid _, void* data, int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal) -> void {
    auto obj1IsPlayer = isPlayer(obj1);
    auto obj2IsPlayer = isPlayer(obj2);
    auto obj1IsPickup = isPickup(obj1);
    auto obj2IsPickup = isPickup(obj2);
    if (obj1IsPlayer && obj2IsPickup){
      tryPickupItem(obj2);
    }else if (obj2IsPlayer && obj1IsPickup){
      tryPickupItem(obj1);
    }
  };

	return binding;
}