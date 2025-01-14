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
};

ManagedSounds& getManagedSounds();
void ensureDefaultSoundsLoadced(objid sceneId);
void ensureSoundsLoaded(objid sceneId, std::string jumpClip, std::string landClip, std::string moveClip);
void ensureSoundsUnloaded(objid sceneId);


struct PrecachedResources {
  std::vector<std::string> models;
  std::vector<objid> ids;
};
void ensurePrecachedModels(objid sceneId, std::vector<std::string> models);

#endif