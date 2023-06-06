#include "./spawn.h"

extern CustomApiBindings* gameapi;

/*
//enemy:tint:1 0 0 1
//enemy:goal:idle-enemy
//enemy:goal-type:idle
//enemy:goal-value:2
//enemy:agent:true
//enemy:script:native/ai
//enemy:health:130
//enemy:team:blue
*/

//////////////////////////////// SPAWNING /////////////////////////////
void createEnemyInstance(objid sceneId, glm::vec3 pos, glm::quat rotation){
  GameobjAttributes attr = {
    .stringAttributes = {
      { "mesh", "../gameresources/build/characters/plaguerobot.gltf" },
      { "physics", "enabled" },
      { "physics_type", "dynamic" },
      { "agent", "true" },
    },
    .numAttributes = {
      { "health", 130.f },
    },
    .vecAttr = {
      .vec3 = {
        { "position", pos },
        { "physics_angle", glm::vec3(0.f, 0.f, 0.f) },
      },
      .vec4 = {

      },
    },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  gameapi -> makeObjectAttr(
    sceneId, 
    "spawned-instance", 
    attr, 
    submodelAttributes
  );
}
void spawnPlayer(objid id){
  auto spawnPosition = gameapi -> getGameObjectPos(id, true);
  auto spawnRotation = gameapi -> getGameObjectRotation(id, true);  // maybe don't want the actual rotn but rather only on xz plane?  maybe?
  createEnemyInstance(gameapi -> listSceneId(id), spawnPosition, spawnRotation);
}

std::unordered_map<std::string, int> goalnameToInt;
int symbolIndex = -1;
int getSymbol(std::string name){
  if (goalnameToInt.find(name) != goalnameToInt.end()){
    return goalnameToInt.at(name);
  }
  symbolIndex++;
  goalnameToInt[name] = symbolIndex;
  return symbolIndex;
} 
std::string nameForSymbol(int symbol){
  for (auto &[name, symbolIndex] : goalnameToInt){
    if (symbolIndex == symbol){
      return name;
    }
  }
  modassert(false, "could not find symbol for: " + symbol);
  return "";
}
