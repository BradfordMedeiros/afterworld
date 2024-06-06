#ifndef MOD_AFTERWORLD_CUTSCENE
#define MOD_AFTERWORLD_CUTSCENE

#include "./util.h"

struct CutsceneApi {
	std::function<void(std::string title, float duration)> showLetterBox;
	std::function<void(glm::vec3 position, glm::quat rotation)> setCameraPosition;
	std::function<void()> popTempViewpoint;
};

void playCutscene(std::string&& cutsceneName, float startTime);
void tickCutscenes(CutsceneApi& api, float time);
void onCutsceneMessages(std::string& key);

#endif