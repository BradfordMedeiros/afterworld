#include "./progress.h"

extern CustomApiBindings* gameapi;
extern std::vector<CrystalPickup> crystals;   // static-state extern

const char* CRYSTAL_SAVE_FILE = "../afterworld/data/save/save.json";

bool hasCrystal(std::unordered_map<std::string, std::unordered_map<std::string, JsonType>>& values, std::string key){
  if (values.find("crystals") == values.end()){
    return false;
  }
  for (auto &[savedKey, savedValueObj] : values.at("crystals")){
    auto boolValue = std::get_if<bool>(&savedValueObj);
    if (boolValue){
      if (savedKey == (std::string(key) + std::string("|") + std::string("has_crystal"))) {
        return *boolValue;
      }      
    }
  }
  return false;
}

std::vector<CrystalPickup> loadCrystals(){
  bool success = false;
  auto data = gameapi -> loadFromJsonFile (CRYSTAL_SAVE_FILE, &success);
  if (!success){
    modassertwarn(false, "load save file failed");
    data = {};
  }

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
      crystalValues[key] = true;
    }
  }
  persistSaveMap("crystals", crystalValues);
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