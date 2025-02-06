#ifndef MOD_AFTERWORLD_RESOURCES
#define MOD_AFTERWORLD_RESOURCES

#include <string>
#include <optional>
#include "./util.h"

struct ManagedSounds {
  std::optional<objid> jumpSoundObjId;
  std::optional<objid> landSoundObjId;
  std::optional<objid> moveSoundObjId;
  std::optional<objid> activateSoundObjId;
  std::optional<objid> soundObjId;
  std::optional<objid> explosionSoundObjId;

  std::unordered_map<objid, std::vector<objid>> sceneIdToSounds;
  std::unordered_map<objid, std::vector<objid>> sceneIdToTextures;
};

ManagedSounds& getManagedSounds();
void ensureDefaultSoundsLoadced(objid sceneId);
void ensureSoundsLoaded(objid sceneId, std::string jumpClip, std::string landClip, std::string moveClip);
void ensureSoundsUnloaded(objid sceneId);

// these dont actually assume it's a scene id
std::vector<objid> ensureSoundLoadedBySceneId(objid id, objid sceneId, std::vector<std::string>& soundsToLoad);
void unloadManagedSounds(objid id);

void ensureManagedTexturesLoaded(objid id, objid sceneId, std::vector<std::string> textures);
void unloadManagedTexturesLoaded(objid id);

struct PrecachedResources {
  std::vector<std::string> models;
  std::vector<objid> ids;
};
void ensurePrecachedModels(objid sceneId, std::vector<std::string> models);

#endif