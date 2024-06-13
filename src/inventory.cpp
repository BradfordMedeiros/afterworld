#include "./inventory.h"

extern CustomApiBindings* gameapi;

std::unordered_map<int, float> inventoryCount {};

int currentItemCount(std::string name){
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


void setGunAmmo(std::string gun, int currentAmmo){
  updateItemCount(gun + "-ammo", currentAmmo);
}

std::optional<GunInfo> getGunInventoryInfo(std::string gun){
  auto gunInInventory = hasGun(gun);
  if (!gunInInventory){
    return std::nullopt;
  }
  auto currentAmmo = ammoForGun(gun);
  return GunInfo {
    .ammo = currentAmmo,
  };
}
