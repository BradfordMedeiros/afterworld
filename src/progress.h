#ifndef MOD_AFTERWORLD_PROGRESS
#define MOD_AFTERWORLD_PROGRESS

#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"
#include "./global.h"

struct LevelPlaylist {
	std::optional<int> currentProgress;
	std::vector<std::string> shortNames;
};

struct GameProgress {
	LevelPlaylist levels;
};

GameProgress& getGameProgress();

void setProgressByShortname(std::string name);
std::optional<std::string> getCurrentLink();
void advanceProgress();
GameProgress& getGameProgress();

#endif