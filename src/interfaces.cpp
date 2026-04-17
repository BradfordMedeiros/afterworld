#include "./interfaces.h"

std::vector<objid> ensureSoundLoadedBySceneId(objid id, objid sceneId, std::vector<std::string>& soundsToLoad);
void unloadManagedSounds(objid id);
void ensureManagedTexturesLoaded(objid id, objid sceneId, std::vector<std::string> textures);
void unloadManagedTexturesLoaded(objid id);

extern MovementEntityData movementEntities;

ArcadeApi createArcadeApi(){
  ArcadeApi arcadeApi {
    .ensureSoundsLoaded = [](objid id, std::vector<std::string> sounds) -> std::vector<objid> {
      return ensureSoundLoadedBySceneId(id, rootSceneId(), sounds);
    },
    .releaseSounds = [](objid id) -> void {
      unloadManagedSounds(id);
    },
    .ensureTexturesLoaded = [](objid id, std::vector<std::string> textures) -> void {
      ensureManagedTexturesLoaded(id, rootSceneId(), textures);
    },
    .releaseTextures = unloadManagedTexturesLoaded,
    .playSound = [](objid clipId) -> void {
      playGameplayClipById(clipId, std::nullopt, std::nullopt, false);
    },
    .getResolution = [](objid id) -> glm::vec2 {
      auto texture = arcadeTextureId(id);
      if (texture.has_value()){
        return glm::vec2(1000, 1000); // this is overly coupled to the create texture call in tags
      }
      auto resolutionAttr = getWorldStateAttr("rendering", "resolution").value();
      glm::vec2* resolution = std::get_if<glm::vec2>(&resolutionAttr);
      modassert(resolution, "resolution value invalid");
      return *resolution;
    }
  };
  return arcadeApi;
}

AIInterface aiInterface {
  .move = [](objid agentId, glm::vec3 targetPosition, float speed) -> void {
    setEntityTargetLocation(movementEntities, agentId, MovementRequest {
      .position = targetPosition,
      .speed = speed * 0.6f,
    });
  },
  .stopMoving = [](objid agentId) -> void {
    setEntityTargetLocation(movementEntities, agentId, std::nullopt);
  },
  .look = [](objid agentId, glm::quat direction) -> void {
    setEntityTargetRotation(movementEntities, agentId, direction);
  },
  .fireGun = [](objid agentId) -> void {
    fireGun(weapons, agentId);
  },
  .changeGun = [](objid agentId, const char* gun) -> void {
    maybeChangeGun(getWeaponState(weapons, agentId), gun,  agentId /*inventory */);
  },
  .changeTraits = [](objid agentId, const char* profile) -> void {
    changeMovementEntityType(movementEntities, agentId, profile);
  },
  .playAnimation = [](objid agentId, const char* animation, AnimationType animationType){
    gameapi -> playAnimation(agentId, animation, animationType, std::nullopt, 0, false, std::nullopt);
  },
  .doDamage = doDamageMessage,
};