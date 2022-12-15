#include "./daynight.h"

extern CustomApiBindings* gameapi;

std::vector<glm::vec3> cycleColors = {
  glm::vec3(0.1f, 0.1f, 0.1f),
  glm::vec3(0.8f, 0.8f, 0.8f),
  glm::vec3(0.8f, 0.8f, 0.8f),
  glm::vec3(0.8f, 0.8f, 0.8f),
  glm::vec3(1.0f, 0.8f, 0.8f),
  glm::vec3(0.8f, 0.8f, 0.8f),
  glm::vec3(0.1f, 0.1f, 0.3f),
  glm::vec3(0.1f, 0.1f, 0.3f),
  glm::vec3(0.1f, 0.1f, 0.1f)
};

float totalDaynightLengthSecs = 20.f * 60.f;  // 20 minutes
float cycleLengthSecs = totalDaynightLengthSecs / cycleColors.size();
float unitCycleLength = cycleLengthSecs / cycleColors.size();

int cycleIndex(float currtime){
  int index = currtime / unitCycleLength;
  return index % cycleColors.size();
}

float calcLerpAmount(float currtime){
  float cycleBeginningTime =  unitCycleLength * static_cast<int>((currtime / unitCycleLength));
  float lerpamount = (currtime - cycleBeginningTime) / unitCycleLength;
  return lerpamount;
}


CScriptBinding daynightBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    return NULL;
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    auto currTime = gameapi -> timeSeconds(false);
    auto currCycle = cycleIndex(currTime);
    auto nextCycle = (currCycle + 1) % cycleColors.size();
    auto lerpamount = calcLerpAmount(currTime);
    auto color = glm::lerp(cycleColors.at(currCycle), cycleColors.at(nextCycle), lerpamount);
    gameapi -> setWorldState({
      ObjectValue {
        .object = "skybox",
        .attribute = "color",
        .value = color,
      },
    });
  };

  return binding;
}

