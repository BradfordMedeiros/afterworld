#include "./spawn.h"

extern CustomApiBindings* gameapi;

//////////////////////////////// SPAWNING /////////////////////////////
void createEnemyInstance(objid sceneId, glm::vec3 pos, glm::quat rotation){
  GameobjAttributes attr = {
    .stringAttributes = {
      { "mesh", "../gameresources/build/characters/plaguerobot.gltf" },
      { "physics", "enabled" },
      { "physics_type", "dynamic" },
    },
    .numAttributes = {},
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