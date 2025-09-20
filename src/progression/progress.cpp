#include "./progress.h"

extern CustomApiBindings* gameapi;

extern std::vector<CrystalPickup> crystals;   // static-state extern

bool hasCrystal(std::unordered_map<std::string, std::unordered_map<std::string, JsonType>>& values, std::string key){
  for (auto &[savedKey, savedValueObj] : values.at("crystals")){
    auto strValue = std::get_if<std::string>(&savedValueObj);
    if (strValue){
      if (savedKey == (std::string(key) + std::string("|") + std::string("has_crystal"))) {
        return *strValue == "true";
      }      
    }
  }
  return false;
}

std::vector<CrystalPickup> loadCrystals(){
  bool success = false;
  auto data = gameapi -> loadFromJsonFile ("./save-file-2.txt", &success);
  modassert(success, "not success");

  return {
    CrystalPickup {
      .hasCrystal = hasCrystal(data, "e1m1"),
      .crystal = Crystal {
        .label = "e1m1",
      },
    },
    CrystalPickup {
      .hasCrystal = hasCrystal(data, "e1m2"),
      .crystal = Crystal {
        .label = "e1m2",
      },
    },
  };
}


void saveCrystals(){
  std::unordered_map<std::string, JsonType> crystalValues;
  for (auto& crystal : crystals){
    std::string key = crystal.crystal.label + std::string("|") + std::string("has_crystal");
    if (crystal.hasCrystal){
      crystalValues[key] = "true";
    }
  }

  std::unordered_map<std::string, std::unordered_map<std::string, JsonType>> values;
  values["crystals"] = crystalValues;
  gameapi -> saveToJsonFile("./save-file-2.txt", values);
}

int numberOfCrystals(){
  int totalCount = 0;
  for (auto& crystal : crystals){
    if (crystal.hasCrystal){
      totalCount++;
    }
  }
  return totalCount;
}
int totalCrystals(){
  return crystals.size();
}

bool hasCrystal(std::string& name){
  for (auto& crystal : crystals){
    if (crystal.hasCrystal && crystal.crystal.label == name){
      return true;
    }
  }
  return false;
}
void pickupCrystal(std::string name){
  bool pickedUp = false;
  for (auto& crystal : crystals){
    if (crystal.crystal.label == name){
      crystal.hasCrystal = true;
      pickedUp = true;
    }
  }
  if (pickedUp){
    modlog("pickupCrystal picked up", name);
  }else{
    modlog("pickupCrystal no matching crystal for", name);
    modassert(false, "bad crystal pickup");
  }
}

void saveData(){
	saveCrystals();
}