#ifndef MOD_AFTERWORLD_SOUNDS
#define MOD_AFTERWORLD_SOUNDS

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"

struct MaterialToSound {
	std::string material;
	std::string clip;
};

struct SoundData {
	std::vector<MaterialToSound> sounds;
};

SoundData createSoundData(objid sceneId);
void onMessageSound(SoundData& sound, int32_t sceneId, std::string& key, std::any& value);
void onCollisionEnterSound(SoundData& sound, int32_t sceneId, int32_t obj1, int32_t obj2, glm::vec3 pos);

#endif 