#include "./progress.h"

extern CustomApiBindings* gameapi;
extern std::vector<CrystalPickup> crystals;   // static-state extern
extern std::vector<LevelProgress> levelProgresses;

const char* PROGRESS_SAVE_FILE = "../afterworld/data/save/save.json";

struct PlaylistType {
  std::string playlist;
  std::string levelShortname;
  std::vector<std::string> crystals;
};

std::vector<PlaylistType> playlist;

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

  std::vector<CrystalPickup> crystalPickups;
  for (auto& item : playlist){
    for (auto &crystal : item.crystals){
      crystalPickups.push_back(CrystalPickup {
        .hasCrystal = hasCrystal(data, crystal),
        .crystal = Crystal {
          .label = crystal,
          .level = item.levelShortname,
        },
      });
    }
  }
  return crystalPickups;
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

bool isCrystalForLevel(CrystalPickup& crystal, std::optional<std::vector<std::string>> levels){
  if (!levels.has_value()){
    return true;
  }
  for (auto& level : levels.value()){
    if (level == crystal.crystal.level){
      return true;
    }
  }
  return true;
}
int numberOfCrystals(std::optional<std::vector<std::string>> levels){
  int totalCount = 0;
  for (auto& crystal : crystals){
    if (!isCrystalForLevel(crystal, levels)){
      continue;
    }
    if (crystal.hasCrystal){
      totalCount++;
    }
  }
  return totalCount;
}
int totalCrystals(std::optional<std::vector<std::string>> levels){
  int count = 0;
  for (auto& crystal : crystals){
    if (!isCrystalForLevel(crystal, levels)){
      continue;
    }
    count++;
  }
  return count;
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

std::vector<PlaylistType> loadPlaylist(){
  auto query = gameapi -> compileSqlQuery("select name, level, crystals from playlist", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  std::vector<PlaylistType> levels = {};
  for (auto &row : result){
    auto playlistName = row.at(0);
    auto levelShortname = row.at(1);
    auto crystalsStr = row.at(2);
    auto crystals = split(crystalsStr, ';');
    levels.push_back(PlaylistType {
      .playlist = playlistName,
      .levelShortname = levelShortname,
      .crystals = crystals,
    });
    std::cout << "playlist: adding: " << levelShortname << ", crystals: " << print(crystals) << std::endl;
  }
  return levels;
}

std::vector<LevelProgress> loadLevelProgress(){
  auto boolValues = getSaveBoolValues("levelprogress", "complete");
  std::vector<LevelProgress> progress;
  for (auto &boolValue : boolValues){
    progress.push_back(LevelProgress {
      .level = boolValue.field,
      .complete = boolValue.value,
    });
  }
  playlist = loadPlaylist(); // maybe this should be separate
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
  return playlist.size();
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

//////////////// ball mode ////////////////////

ProgressInfo getProgressInfo(std::string currentWorld, std::optional<std::string> level, std::vector<std::string> worldLevels){
  std::cout << "getProgressInfo: " << currentWorld << ", " << print(level) << ", " << print(worldLevels) << std::endl;
  ProgressInfo progressInfo {
    .inOverworld = true,
    .worldProgressInfo = WorldProgressInfo{
      .currentWorld = currentWorld,
      .gemCount = numberOfCrystals(worldLevels),
      .totalGemCount = totalCrystals(worldLevels),
    },
    .completedLevels = completedLevels(),
    .totalLevels = totalLevels(),
    .gemCount = numberOfCrystals(std::nullopt),
    .totalGemCount = totalCrystals(std::nullopt),
    .level = std::nullopt,
  };
  if (level.has_value()){
    progressInfo.level = LevelProgressInfo {
      .gemCount = numberOfCrystals(std::vector<std::string>({ level.value() })),
      .totalGemCount = totalCrystals(std::vector<std::string>({ level.value() })),
      .bestTime = 15.f,
      .parTime = 90.f,
    };
  }
  return progressInfo;
}
