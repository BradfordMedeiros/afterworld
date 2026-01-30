#include "./progress.h"

extern CustomApiBindings* gameapi;
extern std::vector<CrystalPickup> crystals;   // static-state extern
extern std::vector<LevelProgress> levelProgresses;

const char* PROGRESS_SAVE_FILE = "../afterworld/data/save/save.json";

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
  auto data = gameapi -> loadFromJsonFile (PROGRESS_SAVE_FILE, &success);
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

//////////////////////////////////

std::vector<LevelProgress> loadLevelProgress(){
  auto boolValues = getSaveBoolValues("levelprogress", "complete");
  std::vector<LevelProgress> progress;
  for (auto &boolValue : boolValues){
    progress.push_back(LevelProgress {
      .level = boolValue.field,
      .complete = boolValue.value,
    });
  }
  return progress;
}

void saveLevelProgress(){
  std::unordered_map<std::string, JsonType> progressValues;
  for (auto& levelProgress : levelProgresses){
    std::string key = levelProgress.level + std::string("|") + std::string("complete");
    if (levelProgress.complete){
      progressValues[key] = true;
    }
  }
  persistSaveMap("levelprogress", progressValues);
}
int completedLevels(){
  int count = 0;
  for (auto& levelProgress : levelProgresses){
    if (levelProgress.complete){
      count++;
    }
  }
  return count;
}
int totalLevels(){
  return levelProgresses.size();
}
void markLevelComplete(std::string name, bool complete){
  modlog("progress markLevelComplete", name + " - " + (complete ? "true" : "false"));
  bool foundLevel = false;
  for (auto& levelProgress : levelProgresses){
    if (levelProgress.level == name){
      foundLevel = true;
      levelProgress.complete = complete;
    }
  }
  if (!foundLevel){
    levelProgresses.push_back(LevelProgress {
      .level = name,
      .complete = complete,
    });    
  }
  saveLevelProgress();
}
bool isLevelComplete(std::string name){
  for (auto& levelProgress : levelProgresses){
    if (levelProgress.level == name && levelProgress.complete){
      return true;
    }
  }
  return false;
}

void resetProgress(){
  levelProgresses = {};
  saveLevelProgress();
}

//////////////////////////

void saveData(){
	saveCrystals();
  saveLevelProgress();
}