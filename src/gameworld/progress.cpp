#include "./progress.h"


extern CustomApiBindings* gameapi;
extern std::vector<LevelProgress> levelProgresses;
const char* PROGRESS_SAVE_FILE = "../afterworld/data/save/save.json";


std::vector<Playlist> playlists;   // TODO STATIC
std::set<std::string> stagedCrystals; // TODO STATIC


LevelConditionData levelConditionData{};

LevelConditionData getConditionData(){
  auto triggers = getSaveVectorValue("condition", "triggers");
  levelConditionData.triggers = triggers;
  return levelConditionData;
}

void saveConditions(LevelConditionData data){
    levelConditionData = data;

    std::unordered_map<std::string, JsonType> values;

    std::vector<std::string> strValues;
    for (auto& trigger : data.triggers){
      strValues.push_back(trigger);
    }

    values["triggers"] = strValues;
    persistSaveMap("condition", values);
}


Playlist parsePlaylist(std::string filepath){
  Playlist playlist{};

  auto fileInfo = decomposePath(filepath);
  auto relativeDir = relativePath("../afterworld/data/config/playlists/ ", fileInfo.dirPath, ".");
  auto relativeDirVec = split(relativeDir, '/');

  std::string name = fileInfo.filename;

  auto fileContent = readFileOrPackage(filepath);
  rapidjson::Document doc;
  rapidjson::ParseResult ok = doc.Parse(fileContent.c_str());
  if (doc.HasParseError()){
    std::cout << "error parsing game file: " << filepath << "  (" << fileContent << ")" << std::endl;
  }
 

  std::vector<PlaylistLevel> playlistLevels;
  {
    auto it = doc.FindMember("levels");
    if (it != doc.MemberEnd() && it -> value.IsArray()) {
        for (auto& item : it -> value.GetArray()) {
            if (!item.IsObject()) {
              continue;
            }

            PlaylistLevel playlistLevel{};
            {
              auto levelIt = item.FindMember("level");
              if (levelIt != item.MemberEnd() && levelIt -> value.IsString()) {
                  playlistLevel.level = levelIt -> value.GetString();
              }
            }
            {
              auto worldIt = item.FindMember("world");
              if (worldIt != item.MemberEnd() && worldIt -> value.IsString()) {
                  playlistLevel.world = worldIt -> value.GetString();
              }
            }
            {
              auto parTimeIt = item.FindMember("par");
              if (parTimeIt != item.MemberEnd() && parTimeIt -> value.IsNumber()) {
                  playlistLevel.parTime = parTimeIt -> value.GetDouble();
              }
            }
            playlistLevels.push_back(playlistLevel);
        }
    }
  }


  return Playlist{ 
    .name = name,
    .levels = playlistLevels,
  };
}

std::vector<Playlist>  loadPlaylists(){
  std::vector<Playlist> playlists;
  auto playlistFiles = listFilesWithExtensionsFromPackage("../afterworld/data/config/playlists", { "json" });
  for (auto& playlistFile : playlistFiles){
    auto playlist = parsePlaylist(playlistFile);
    playlists.push_back(playlist);
  }

  return playlists;
}

std::vector<std::string> worldsForPlaylist(std::string playlistName){
  std::vector<std::string> worlds;
  for (auto& playlist : playlists){
    if (playlist.name == playlistName){
      for (auto& level : playlist.levels){
        bool foundAlready = false;
        for (auto& world : worlds){
          if (level.world == world){
            foundAlready = true;
            break;
          }
        }
        if (!foundAlready){
          worlds.push_back(level.world);
        }
      }
      return worlds;
    }
  }
  modassert(false, "worldsForPlaylist no specified playlist exists");
  return {};
}

std::optional<Playlist*> playlistByName(std::string playlistName){
  for (auto& playlist : playlists){
    if (playlist.name == playlistName){
      return &playlist;
    }
  }
  return std::nullopt;
}

std::vector<PlaylistLevel*> levelsForWorld(Playlist& playlist, std::string world){
  std::vector<PlaylistLevel*> levels;
  for (auto& playlistLevel : playlist.levels){
    if (playlistLevel.world == world){
      levels.push_back(&playlistLevel);
    }
  }
  return levels;
}


std::string print(PlaylistLevel& playlistLevel){
  std::string value;
  value += "[level = ";
  value += playlistLevel.level;
  value += ", world = ";
  value += playlistLevel.world; 
  value += ", parTime = ";
  value += playlistLevel.parTime.has_value() ? std::to_string(playlistLevel.parTime.value()) : "n/a"; 
  value += " ]";

  return value;
}

std::string print(Playlist& playlist){
  std::string value;
  value += "playlist = ";
  value += playlist.name;
  value += " ( ";
  for (auto& playlistLevel : playlist.levels){
    value += print(playlistLevel) + "  ";
  }
  value += ")";
  return value;
}


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
      std::cout << "numberOfCrystals adding for: " << levelProgress.level << ", size = " << levelProgress.crystals.size() << std::endl;
      count += levelProgress.crystals.size();
    }else{
      auto& levelsToCount = levels.value();
      for (auto& level : levelsToCount){
        if (levelProgress.level == level){
          std::cout << "numberOfCrystals adding for: " << level << ", size = " << levelProgress.crystals.size() << std::endl;
          count += levelProgress.crystals.size();
          break;
        }
      }
    }
  }
  return count;
}
int totalCrystals(std::optional<std::vector<std::string>> levels){
  /*int count = 0;
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
  return count;*/
  return 0;
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

  bool foundLevelForCrystal = false;
  for (auto& playlist : playlists){
    for (auto& level : playlist.levels){
      auto levelProgress = getLevelProgress(level.level);
      modassert(levelProgress.has_value(), std::string("level progress no value: ") + level.level);
      if (level.crystals.count(name) > 0){
        std::cout << "progress pickup added to " << level.level << std::endl;
        levelProgress.value() -> crystals.insert(name);
        foundLevelForCrystal = true;
      }
    }
  }

  if (!foundLevelForCrystal){
    std::cout << "progress pickup - missing level for crystal: " << name << std::endl;
  }

  saveLevelProgress();
}

void stageCrystal(std::string name){
  std::cout << "progress stageCrystal: " << name << std::endl;
  stagedCrystals.insert(name);
}
void commitCrystals(){
  for (auto crystal : stagedCrystals){
    pickupCrystal(crystal);
  }
  std::cout << "progress commitCrystals" << std::endl;
  stagedCrystals = {};
}

std::vector<LevelProgress> loadLevelProgress(){
  playlists = loadPlaylists();;

  std::unordered_map<std::string, LevelProgress> levelToLevelProgress;
  for (auto& playlist : playlists){
    for (auto& playlistLevel : playlist.levels){
      if (levelToLevelProgress.find(playlistLevel.level) == levelToLevelProgress.end()){
        levelToLevelProgress[playlistLevel.level] = LevelProgress {
          .level = playlistLevel.level,
          .complete = false,
          .bestTime = std::nullopt,
          .crystals = {},
        };
      }
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
  int count = 0;
  for (auto& playlist : playlists){
    count += playlist.levels.size();
  }
  return count;
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
  for (auto& playlist : playlists){
    for (auto& playlistLevel : playlist.levels){
      if (levelToLevelProgress.find(playlistLevel.level) == levelToLevelProgress.end()){
        levelToLevelProgress[playlistLevel.level] = LevelProgress {
          .level = playlistLevel.level,
          .complete = false,
          .bestTime = std::nullopt,
          .crystals = {},
        };
      }
    }
  }
  for (auto& [_, levelProgress] : levelToLevelProgress){
    levelProgresses.push_back(levelProgress);
  }

  // 
  levelConditionData = LevelConditionData{};
  saveConditions(levelConditionData);

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
  for (auto& playlist : playlists){
    for (auto& playlistLevel : playlist.levels){
      if (playlistLevel.level == level){
        return playlistLevel.parTime.has_value() ? playlistLevel.parTime.value() : 0.f;
      }
    }    
  }
  return 0.f;
}

//////////////// ball mode ////////////////////
std::vector<std::string> playlistLevelsInWorld(std::string world){
  std::vector<std::string> levels;
  for (auto& playlist : playlists){
    for (auto& playlistType : playlist.levels){
      if (playlistType.world == world){
        levels.push_back(playlistType.level);
      }
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


std::vector<RawLevelData> getRawLevelData(){
  std::vector<RawLevelData> levelData;

  auto extraMaps = listFilesWithExtensionsFromPackage("../afterworld/scenes/levels/worlds/", { "rawscene" });
  for (auto& rawsceneFile : extraMaps){
    auto filePathData = decomposePath(rawsceneFile);
    auto levelPathData = decomposePath(filePathData.dirPath);
    auto worldPathData = decomposePath(levelPathData.dirPath);

    auto levelName = levelPathData.filename;
    auto worldName = worldPathData.filename;
 
    auto imageName = filePathData.dirPath + "/map.png";
    auto image = fileExistsFromPackage(imageName) ? imageName : "./res/textures/wood.jpg";
    std::cout << "dyn image: " << imageName << ", exists = " << fileExistsFromPackage(imageName) << std::endl;

    auto mapName = filePathData.dirPath + "/map.map";
    bool mapExists = fileExistsFromPackage(mapName);
    std::cout << "dyn map: " << mapName << ", exists = " << mapExists << std::endl;
      
    std::optional<std::string> additionalSceneFilepath = filePathData.dirPath + "/items.rawscene2";
    bool additionalSceneExists = fileExistsFromPackage(additionalSceneFilepath.value());
    if (!additionalSceneExists){
      additionalSceneFilepath = std::nullopt;
    }

    std::string configFile = filePathData.dirPath + "/config.json";
    bool configExists = fileExistsFromPackage(configFile);
    

    glm::vec3 ambientLight(0.4f, 0.4f, 0.4f); 
    glm::vec3 skyboxColor(1.f, 1.f, 1.f);
    std::string skybox("../gameresources/skybox/storm");
    std::string description("[no description]");
    std::string mode("ball");
    std::optional<std::string> weather;
    glm::vec2 chromatic(0.f, 0.f);

    if (configExists){
      bool success = true;
      auto data = gameapi -> loadFromJsonFile2(configFile, &success, false);
      modassert(success, "error parsing json");

      if (data.find("ambient") != data.end()){
        auto ambientPtr = std::get_if<std::vector<float>>(&data.at("ambient"));
        modassert(ambientPtr -> size() == 3, std::string("unexpected ambientPtr value, got size = ") + std::to_string(ambientPtr -> size()));
        ambientLight = glm::vec3(ambientPtr -> at(0), ambientPtr -> at(1), ambientPtr -> at(2));
      }
      if (data.find("skyboxcolor") != data.end()){
        auto skyboxColorPtr = std::get_if<std::vector<float>>(&data.at("skyboxcolor"));
        modassert(skyboxColorPtr -> size() == 3, std::string("unexpected skybox value, got size = ") + std::to_string(skyboxColorPtr -> size()));
        skyboxColor = glm::vec3(skyboxColorPtr -> at(0), skyboxColorPtr -> at(1), skyboxColorPtr -> at(2));
      }
      if (data.find("skybox") != data.end()){
        auto skyboxPtr = std::get_if<std::string>(&data.at("skybox"));
        skybox = *skyboxPtr; 
      }
      if (data.find("description") != data.end()){
        auto descriptionPtr = std::get_if<std::string>(&data.at("description"));
        description = *descriptionPtr;
      }
      if (data.find("mode") != data.end()){
        auto modePtr = std::get_if<std::string>(&data.at("mode"));
        mode = *modePtr;
      }
      if (data.find("weather") != data.end()){
        auto weatherPtr = std::get_if<std::string>(&data.at("weather"));
        weather = *weatherPtr;   
      }
      if (data.find("mode") != data.end()){
        auto modePtr = std::get_if<std::string>(&data.at("mode"));
        mode = *modePtr;
      }

      if (data.find("chromatic") != data.end()){
        auto chromaticPtr = std::get_if<std::vector<float>>(&data.at("chromatic"));
        modassert(chromaticPtr -> size() == 2, std::string("unexpected chromaticPtr value, got size = ") + std::to_string(chromaticPtr -> size()));
        chromatic = glm::vec2(chromaticPtr -> at(0), chromaticPtr -> at(1));
      }

    }


    levelData.push_back(RawLevelData {
      .name = levelPathData.filename,
      .world = worldName,
      .filepath = rawsceneFile,
      .additionalFilepath = additionalSceneFilepath,
      .description = description,
      .image = image,
      .shortcut = levelPathData.filename,
      .ambientLight = ambientLight,
      .skyboxColor = skyboxColor,
      .skybox = skybox,
      .weather = weather,
      .audioClipPath = "../gameresources/sound/rain.wav",
      .mode = mode,
      .chromatic = chromatic,
      .additionalTokens = {},

      .configFile = configFile,   
      .configFileExists = configExists,
    });
  }


  return levelData;
}

std::optional<RawLevelData> levelByShortcutName(std::string shortcut){
  static auto rawLevels = getRawLevelData();
  for (auto& rawLevel : rawLevels){
    if (rawLevel.shortcut == shortcut){
      return rawLevel;
    }
  }
  return std::nullopt;
}

std::optional<RawLevelData> levelByName(std::string levelName){
  static auto allLevels = getRawLevelData();
  for (auto& level : allLevels){
    if (level.name == levelName){
      return level;
    }
  }
  return std::nullopt;
}

void updateRawLevelData(std::string levelName, UpdateLevel updateLevel){
  auto oldLevelData = levelByName(levelName);
  if (oldLevelData.has_value()){
    auto level = oldLevelData.value();
  
    std::unordered_map<std::string, JsonType> data;
    if (level.configFileExists){
      bool success = true;
      data = gameapi -> loadFromJsonFile2(level.configFile, &success, false);
      modassert(success, "error parsing json");
    }

    if (updateLevel.description.has_value()){
      data["description"] = updateLevel.description.value();
    }
    if (updateLevel.skybox.has_value()){
      data["skybox"] = updateLevel.skybox.value();
    }

    if (updateLevel.ambient.has_value()){
      data["ambient"] = std::vector<float>({ updateLevel.ambient.value().r,  updateLevel.ambient.value().g, updateLevel.ambient.value().b });
    }

    if (updateLevel.chromatic.has_value()){
      data["chromatic"] = std::vector<float>({ updateLevel.chromatic.value().x,  updateLevel.chromatic.value().y });
    }

    if (updateLevel.skyboxColor.has_value()){
      data["skyboxcolor"] = std::vector<float>({ updateLevel.skyboxColor.value().r,  updateLevel.skyboxColor.value().g, updateLevel.skyboxColor.value().b });
    }

    if (updateLevel.weather.has_value()){
      data["weather"] =  updateLevel.weather.value();
    }else{
      data["weather"] = "default";
    }

    gameapi -> saveToJsonFile2(level.configFile, data);
  }
}

std::string print(RawLevelData& level){
  std::string value;
  value += "[ name = " + level.name + " ]";
  return value;
}

std::string print(std::vector<RawLevelData>& levels){
  std::string value;
  for (auto& level : levels){
    value += print(level) + " ";
  }
  return value;
}