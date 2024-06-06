#ifndef MOD_AFTERWORLD_CUTSCENE
#define MOD_AFTERWORLD_CUTSCENE

#include "./util.h"

struct CutsceneApi {
	std::function<void(std::string title, float duration)> showLetterBox;

};

void playCutscene(std::string&& cutsceneName, float startTime);
void tickCutscenes(CutsceneApi& api, float time);

#endif