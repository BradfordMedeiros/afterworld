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

#endif