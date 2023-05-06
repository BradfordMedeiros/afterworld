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

CScriptBinding inventoryBinding(CustomApiBindings& api, const char* name){
	 auto binding = createCScriptBinding(name, api);
   binding.onMessage = [](int32_t id, void* data, std::string& key, AttributeValue& value){
   	if (key == "selected"){  // maybe this logic should be somewhere else and not be in dialog
      auto strValue = std::get_if<std::string>(&value); 
      modassert(strValue != NULL, "selected value invalid");
      auto gameObjId = std::atoi(strValue -> c_str());
      if (!gameapi -> getGameObjNameForId(gameObjId).has_value()){
        return;
      }

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

 		    //gameapi -> removeObjectById(gameObjId);
  			// fake delete because of bug need to fix.  Obviously can't stay like this
 			  gameapi -> setGameObjectPosition(gameObjId, glm::vec3(0.f, -100.f, 0.f), false);

 			  if (pickupTrigger.has_value()){
 			  	gameapi -> sendNotifyMessage(pickupTrigger.value(), std::to_string(newItemCount));
 			  }
  		}
    }

    if (key == "request-change-gun"){
      auto strValue = std::get_if<std::string>(&value); 
      modassert(strValue != NULL, "selected value invalid");
      if (hasGun(*strValue)){
        gameapi -> sendNotifyMessage("change-gun", *strValue);
      }
    }
  };
	return binding;
}