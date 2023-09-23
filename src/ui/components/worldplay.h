#ifndef MOD_AFTERWORLD_COMPONENTS_WORLDPLAY
#define MOD_AFTERWORLD_COMPONENTS_WORLDPLAY

#include "./imagelist.h"
#include "./basic/layout.h"

struct WorldPlayInterface {
	std::function<bool()> isGameMode;
	std::function<bool()> isPaused;
	std::function<void()> enterGameMode;
	std::function<void()> exitGameMode;
	std::function<void()> pause;
	std::function<void()> resume;
	std::function<void()> saveScene;
};

extern Component worldplay;

#endif