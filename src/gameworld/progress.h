#ifndef MOD_AFTERWORLD_PROGRESS
#define MOD_AFTERWORLD_PROGRESS

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "../global.h"
#include "./save.h"

int numberOfCrystals(std::optional<std::vector<std::string>> levels);
int totalCrystals(std::optional<std::vector<std::string>> levels);
void stageCrystal(std::string name);
void commitCrystals();
bool hasCrystal(std::string& name);



////////////////////////////////////////////////////

struct LevelProgress {
  std::string level;
  bool complete;
  std::optional<float> bestTime;
  std::set<std::string> crystals;
};
std::vector<LevelProgress> loadLevelProgress();
void saveLevelProgress();
int completedLevels();
void markLevelComplete(std::string name, float time);
bool isLevelComplete(std::string name);

void resetProgress();

//////////////

void saveData();

////////////////////
// ball mode


struct PlaylistLevel {
  std::string level;
  std::string world;
  std::optional<float> parTime;
  std::set<std::string> crystals;
  bool mustUnlock = true;
};
struct Playlist {
  std::string name;
  std::vector<PlaylistLevel> levels;
};

std::vector<std::string> worldsForPlaylist(std::string playlistName);
std::optional<Playlist*> playlistByName(std::string playlistName);
std::vector<PlaylistLevel*> levelsForWorld(Playlist& playlist, std::string world);
std::optional<PlaylistLevel*> levelInPlaylist(Playlist& playlist, std::string levelName);

struct PlaylistProgressInfo {
  std::string currentWorld;
  int completedLevels;
  int totalLevels;
  int gemCount;
  int totalGemCount;
};

PlaylistProgressInfo getPlaylistProgressInfo();


struct WorldProgressInfo {
  std::string currentWorld;
  int gemCount;
  int totalGemCount;
};
WorldProgressInfo getWorldProgressInfo(std::string currentWorld);


struct LevelProgressInfo {
  int gemCount;
  int totalGemCount;
  std::optional<float> bestTime;
  float parTime;
};
LevelProgressInfo getLevelProgressInfo(std::string currentWorld, std::string level);



struct LevelConditionData {
  std::vector<std::string> triggers;
};
LevelConditionData getConditionData();
void saveConditions(LevelConditionData levelConditionData);


struct RawLevelData {
  std::string name;
  std::string world;
  std::string filepath;
  std::optional<std::string> additionalFilepath;
  std::string description;
  std::string image;
  std::string shortcut;
  glm::vec3 ambientLight;
  glm::vec3 skyboxColor;
  std::string skybox;
  std::optional<std::string> weather;
  std::string audioClipPath;
  std::string mode;
  glm::vec2 chromatic;

  std::vector<std::vector<std::string>> additionalTokens;

  std::string configFile;
  bool configFileExists;
};
std::vector<RawLevelData> getRawLevelData();
std::optional<RawLevelData> levelByShortcutName(std::string shortcut);


struct UpdateLevel {
  std::optional<std::string> skybox;
  std::optional<std::string> description;
  std::optional<glm::vec3> ambient;
  std::optional<glm::vec3> skyboxColor;
  std::optional<std::string> weather;
  std::optional<glm::vec2> chromatic;
};
void updateRawLevelData(std::string levelName, UpdateLevel updateLevel);
std::string print(std::vector<RawLevelData>& levels);


#endif