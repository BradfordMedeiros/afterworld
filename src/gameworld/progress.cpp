#include "./progress.h"


extern CustomApiBindings* gameapi;
extern std::vector<LevelProgress> levelProgresses;
const char* PROGRESS_SAVE_FILE = "../afterworld/data/save/save.json";

bool isControlledVehicle(int vehicleId);


struct PlaylistType {
  std::string playlist;
  std::string levelShortname;
  std::set<std::string> crystals;
  std::optional<float> parTime;
  std::optional<std::string> world;
};

std::vector<PlaylistType> playlist;   // TODO STATIC
std::set<std::string> stagedCrystals; // TODO STATIC

std::optional<LevelProgress*> getLevelProgress(std::string level){
  for (auto& levelProgress : levelProgresses){
    if (levelProgress.level == level){
      return &levelProgress;
    }
  }
  return std::nullopt;
}


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

int numberOfCrystals(std::optional<std::vector<std::string>> levels){
  int count = 0;
  for (auto& levelProgress : levelProgresses){
    if (!levels.has_value()){
      count += levelProgress.crystals.size();
    }else{
      auto& levelsToCount = levels.value();
      for (auto& level : levelsToCount){
        if (levelProgress.level == level){
          count += levelProgress.crystals.size();
          break;
        }
      }
    }
  }
  return count;
}
int totalCrystals(std::optional<std::vector<std::string>> levels){
  int count = 0;
  for (auto& levelProgress : playlist){
    if (!levels.has_value()){
      count += levelProgress.crystals.size();
    }else{
      auto& levelsToCount = levels.value();
      for (auto& level : levelsToCount){
        if (levelProgress.levelShortname == level){
          count += levelProgress.crystals.size();
          break;
        }
      }
    }
  }
  return count;
}

bool hasCrystal(std::string& name){
  for (auto& levelProgress : levelProgresses){
    if (levelProgress.crystals.count(name) > 0){
      return true;
    }
  }
  return false;
}

void pickupCrystal(std::string name){
  std::cout << "pickup crystal: " << name << std::endl;
  for (auto& level : playlist){
    auto levelProgress = getLevelProgress(level.levelShortname);
    modassert(levelProgress.has_value(), std::string("level progress no value: ") + level.levelShortname);
    if (level.crystals.count(name) > 0){
      levelProgress.value() -> crystals.insert(name);
    }
  }

  saveLevelProgress();
}

void stageCrystal(std::string name){
  stagedCrystals.insert(name);
}
void commitCrystals(){
  for (auto crystal : stagedCrystals){
    pickupCrystal(crystal);
  }
  stagedCrystals = {};
}

void handleGemCollision(int32_t obj1, int32_t obj2){
  if (isControlledVehicle(obj1)){
    auto objAttr = getAttrHandle(obj2);
    auto gem = getStrAttr(objAttr, "gem");
    if (gem.has_value()){
      stageCrystal(gem.value());
      gameapi -> removeObjectById(obj2);
    }
  }else if (isControlledVehicle(obj2)){
    auto objAttr = getAttrHandle(obj1);
    auto gem = getStrAttr(objAttr, "gem");
    if (gem.has_value()){
      stageCrystal(gem.value());
      gameapi -> removeObjectById(obj1);
    }
  } 
}


std::vector<PlaylistType> loadPlaylist(){
  auto query = gameapi -> compileSqlQuery("select name, level, crystals, par, world from playlist", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  std::vector<PlaylistType> levels = {};
  for (auto &row : result){
    auto playlistName = row.at(0);
    auto levelShortname = row.at(1);

    auto crystalsStr = row.at(2);
    auto crystalsVec = split(crystalsStr, ';');
    std::set<std::string> crystals;
    for (auto& crystal : crystalsVec){
      crystals.insert(crystal);
    }

    auto parTimeStr = row.at(3);
    auto parTime = parTimeStr == "" ? std::optional<float>(std::nullopt) : std::atof(parTimeStr.c_str());
    levels.push_back(PlaylistType {
      .playlist = playlistName,
      .levelShortname = levelShortname,
      .crystals = crystals,
      .parTime = parTime,
      .world = (row.at(4) == "" ? std::optional<std::string>(std::nullopt) : row.at(4)),
    });
    std::cout << "playlist: adding: " << levelShortname << ", crystals: " << print(crystals) << std::endl;
  }
  return levels;
}

std::vector<LevelProgress> loadLevelProgress(){
  playlist = loadPlaylist(); // maybe this should be separate

  std::unordered_map<std::string, LevelProgress> levelToLevelProgress;
  for (auto& playlistLevel : playlist){
    if (levelToLevelProgress.find(playlistLevel.levelShortname) == levelToLevelProgress.end()){
      levelToLevelProgress[playlistLevel.levelShortname] = LevelProgress {
        .level = playlistLevel.levelShortname,
        .complete = false,
        .bestTime = std::nullopt,
        .crystals = {},
      };
    }
  }


  auto boolValues = getSaveBoolValues("levelprogress", "complete");
  auto floatValues = getSaveFloatValues("levelprogress", "bestTime");
  auto vecStrValues = getSaveVecStrValues("levelprogress", "crystals");

  for (auto &boolValue : boolValues){
    levelToLevelProgress[boolValue.field] = LevelProgress {
      .level = boolValue.field,
      .complete = boolValue.value,
      .bestTime = std::nullopt,
      .crystals = {},
    };
  }

  for (auto &floatValue : floatValues){
    if (levelToLevelProgress.find(floatValue.field) == levelToLevelProgress.end()){
      levelToLevelProgress[floatValue.field] = LevelProgress {
        .level = floatValue.field,
        .complete = false,
        .bestTime = floatValue.value,
        .crystals = {},
      };
    }else{
      levelToLevelProgress.at(floatValue.field).bestTime = floatValue.value; 
    }
  }

  for (auto &vecStr : vecStrValues){
    std::set<std::string> crystals;
    for (auto& value : vecStr.value){
      crystals.insert(value);
    }
    if (levelToLevelProgress.find(vecStr.field) == levelToLevelProgress.end()){
      levelToLevelProgress[vecStr.field] = LevelProgress {
        .level = vecStr.field,
        .complete = false,
        .bestTime = std::nullopt,
        .crystals = crystals,
      };
    }else{
      levelToLevelProgress.at(vecStr.field).crystals = crystals; 
    }
  }

  std::vector<LevelProgress> progress;
  for (auto& [_, levelProgress] : levelToLevelProgress){
    progress.push_back(levelProgress);
  }



  return progress;
}

void saveLevelProgress(){
  std::unordered_map<std::string, JsonType> progressValues;
  for (auto& levelProgress : levelProgresses){
    if (levelProgress.complete){
      std::string levelCompleteKey = levelProgress.level + std::string("|") + std::string("complete");
      progressValues[levelCompleteKey] = true;
    }
    if (levelProgress.bestTime.has_value()){
      std::string bestTimeKey = levelProgress.level + std::string("|") + std::string("bestTime");
      progressValues[bestTimeKey] = levelProgress.bestTime.value();
    }

    if (levelProgress.crystals.size() > 0){
      std::string crystalsKey = levelProgress.level + std::string("|") + std::string("crystals");
      std::vector<std::string> crystalsVec;
      for (auto& crystal : levelProgress.crystals){
        crystalsVec.push_back(crystal);
      }
      progressValues[crystalsKey] = crystalsVec;
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

void markLevelComplete(std::string name, float time){
  modlog("progress markLevelComplete", name + " - " + std::to_string(time));
  bool foundLevel = false;
  for (auto& levelProgress : levelProgresses){
    if (levelProgress.level == name){
      foundLevel = true;
      levelProgress.complete = true;
      if (!levelProgress.bestTime.has_value() || levelProgress.bestTime.value() > time){
        levelProgress.bestTime = time;
      }
    }
  }
  if (!foundLevel){
    levelProgresses.push_back(LevelProgress {
      .level = name,
      .complete = true,
      .bestTime = time,
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
  std::unordered_map<std::string, LevelProgress> levelToLevelProgress;
  for (auto& playlistLevel : playlist){
    if (levelToLevelProgress.find(playlistLevel.levelShortname) == levelToLevelProgress.end()){
      levelToLevelProgress[playlistLevel.levelShortname] = LevelProgress {
        .level = playlistLevel.levelShortname,
        .complete = false,
        .bestTime = std::nullopt,
        .crystals = {},
      };
    }
  }
  for (auto& [_, levelProgress] : levelToLevelProgress){
    levelProgresses.push_back(levelProgress);
  }


  saveLevelProgress();
}

//////////////////////////

void saveData(){
  saveLevelProgress();
}

std::optional<float> bestTime(std::string& level){
  auto levelProgress = getLevelProgress(level);
  if (!levelProgress.has_value()){
    return std::nullopt;
  }
  return levelProgress.value() -> bestTime;
}

float parTime(std::string& level){
  for (auto& playlistLevel : playlist){
    if (playlistLevel.levelShortname == level){
      return playlistLevel.parTime.has_value() ? playlistLevel.parTime.value() : 0.f;
    }
  }
  return 0.f;
}

//////////////// ball mode ////////////////////
std::vector<std::string> playlistLevelsInWorld(std::string world){
  std::vector<std::string> levels;
  for (auto& playlistType : playlist){
    if (playlistType.world == world){
      levels.push_back(playlistType.levelShortname);
    }
  }
  return levels;
}

PlaylistProgressInfo getPlaylistProgressInfo(){
  PlaylistProgressInfo progressInfo {
    .completedLevels = completedLevels(),
    .totalLevels = totalLevels(),
    .gemCount = numberOfCrystals(std::nullopt),
    .totalGemCount = totalCrystals(std::nullopt),
  };
  return progressInfo;
}

WorldProgressInfo getWorldProgressInfo(std::string currentWorld){
  auto worldLevels = playlistLevelsInWorld(currentWorld);
  WorldProgressInfo worldProgressInfo {
    .currentWorld = currentWorld,
    .gemCount = numberOfCrystals(worldLevels),
    .totalGemCount = totalCrystals(worldLevels),
  };
  return worldProgressInfo;
}

LevelProgressInfo getLevelProgressInfo(std::string currentWorld, std::string level){
  LevelProgressInfo levelProgressInfo {
    .gemCount = numberOfCrystals(std::vector<std::string>({ level })),
    .totalGemCount = totalCrystals(std::vector<std::string>({ level })),
    .bestTime = bestTime(level),
    .parTime = parTime(level),
  };
  return levelProgressInfo;
}
