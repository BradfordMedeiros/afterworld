#include "./daynight.h"

extern CustomApiBindings* gameapi;

std::vector<glm::vec3> cycleColors = {
  glm::vec3(0.1f, 0.1f, 0.1f),
  glm::vec3(0.1f, 0.1f, 0.1f),
  glm::vec3(0.1f, 0.1f, 0.1f),
  glm::vec3(0.1f, 0.1f, 0.1f),
  glm::vec3(1.0f, 0.8f, 0.8f),
  glm::vec3(1.0f, 0.8f, 0.8f),
  glm::vec3(0.1f, 0.1f, 0.1f),
  glm::vec3(0.1f, 0.1f, 0.1f),
  glm::vec3(0.1f, 0.1f, 0.1f),

};

const float totalMinutes = 24 * 60;
const float minutesPerCycleState = totalMinutes / cycleColors.size();
const bool useRealTime = true;
const float maxMinutePerDay = 60 * 24;

void spawnLight(objid sceneId, glm::vec3 color){
  GameobjAttributes attr {
    .attr = {
      { "position", glm::vec3(0.f, 50.f, 0.f) },
      { "color", color },
      { "voxelsize", 3 },
      { "attenuation", glm::vec3(0.8f, 0.f, 0.f) }
    },
  };

  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  gameapi -> makeObjectAttr(sceneId, "!code-debuglight", attr, submodelAttributes);
}

void maybeSpawnLightFromArgs(){
  auto args = gameapi -> getArgs();
  if (args.find("light") != args.end()){
    auto lightValue = args.at("light");  // on terminal can pass in eg -a "light=1\ 1\ 0"
    glm::vec3 color(1.f, 1.f, 1.f);
    auto isVec3 = maybeParseVec(lightValue, color);
    modassert(isVec3, "invalid value for light: " + lightValue);
    spawnLight(0, color);
  }
}
void onFrameDaynight(){
  return;
  float currMinute;
  if (useRealTime){
    std::time_t t = std::time(0);   // get time now
    std::tm* now = std::localtime(&t);
    currMinute = (now -> tm_hour * 60) + now -> tm_min;
  }else{
    auto currTime = gameapi -> timeSeconds(false);
    currMinute = currTime * 60; // every second is 60 minutes
    currMinute = fmod(currMinute, maxMinutePerDay);
    //std::cout << "Curr time: " << currTime << std::endl;
    //std::cout << "curr minute: " << currMinute << std::endl;
  }

  float currRatio = currMinute / minutesPerCycleState;      
  int lowerIndex = static_cast<int>(currRatio);
  int nextIndex = lowerIndex + 1;
  if (nextIndex >= cycleColors.size()){
    nextIndex = 0;
  }
  float ratioThroughCurrCycle = (currMinute - minutesPerCycleState * lowerIndex) / minutesPerCycleState;
  auto color = glm::lerp(cycleColors.at(lowerIndex), cycleColors.at(nextIndex), ratioThroughCurrCycle);

  modassert(currMinute < maxMinutePerDay, "daynight: cycle minute greater than max in a day");
  modassert(lowerIndex < cycleColors.size(), "daynight: lower index cycle exceeded expected amount");

  gameapi -> setWorldState({
    ObjectValue {
      .object = "skybox",
      .attribute = "color",
      .value = color,
    },
  });
}

