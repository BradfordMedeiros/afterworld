#ifndef MOD_AFTERWORLD_RESOURCES
#define MOD_AFTERWORLD_RESOURCES

#include <string>
#include <optional>
#include "../util.h"
#include "./paths.h"

void ensureManagedTexturesLoaded(objid id, objid sceneId, std::vector<std::string> textures);
void unloadManagedTexturesLoaded(objid id);

struct PrecachedResources {
  std::vector<std::string> models;
  std::vector<objid> ids;
};
void ensurePrecachedModels(objid sceneId, std::vector<std::string> models);

objid createSound(objid sceneId, std::string soundObjName, std::string clip, bool loop);

#endif