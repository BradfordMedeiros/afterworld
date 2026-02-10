#ifndef MOD_AFTERWORLD_PROGRESS
#define MOD_AFTERWORLD_PROGRESS

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "../global.h"
#include "./save.h"

struct Crystal {
  std::string label;
};
struct CrystalPickup {
  bool hasCrystal;
  Crystal crystal;
};

std::vector<CrystalPickup> loadCrystals();
void saveCrystals();
int numberOfCrystals();
int totalCrystals();
void pickupCrystal(std::string name);
bool hasCrystal(std::string& name);


////////////////////////////////////////////////////

struct LevelProgress {
  std::string level;
  bool complete;
};
std::vector<LevelProgress> loadLevelProgress();
void saveLevelProgress();
int completedLevels();
int totalLevels();
void markLevelComplete(std::string name, bool complete);
bool isLevelComplete(std::string name);

void resetProgress();

//////////////

void saveData();

////////////////////
// ball mode

struct LevelProgressInfo {
  std::optional<float> bestTime;
  float parTime;
};
struct ProgressInfo {
  bool inOverworld;
  std::string currentWorld;
  int completedLevels;
  int totalLevels;
  int gemCount;
  int totalGemCount;
  std::optional<LevelProgressInfo> level;
};

ProgressInfo getProgressInfo(std::string currentLevel);


#endif