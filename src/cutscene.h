#ifndef MOD_AFTERWORLD_CUTSCENE
#define MOD_AFTERWORLD_CUTSCENE

#include "./util.h"

struct CutsceneApi {
	std::function<void(std::string title, float duration)> showLetterBox;
	std::function<void(std::optional<std::string> camera, glm::vec3 position, glm::quat rotation, std::optional<float> duration)> setCameraPosition;
	std::function<void(bool)> setPlayerControllable;
	std::function<void()> goToNextLevel;
};

void playCutscene(objid ownerObjId, std::string cutsceneName, float startTime);
void tickCutscenes(CutsceneApi& api, float time);
void onCutsceneMessages(std::string& key);
void onCutsceneObjRemoved(objid id);

#endif