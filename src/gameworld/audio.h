#ifndef MOD_AFTERWORLD_AUDIO
#define MOD_AFTERWORLD_AUDIO

#include <iostream>
#include <vector>
#include <ctime>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../resources/resources.h"

extern std::string defaultAudioClipPath;
void ensureAmbientSound(glm::vec3 cameraPos, int changedLevelFrame, bool hasManagedScene);

#endif 