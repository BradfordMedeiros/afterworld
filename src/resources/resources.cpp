#include "./resources.h"

extern CustomApiBindings* gameapi;

struct ManagedSounds {
  std::unordered_map<objid, std::vector<objid>> sceneIdToTextures;
};
ManagedSounds sounds {
  .sceneIdToTextures = {},
};
PrecachedResources precachedResources {
  .models = {},
  .ids = {},
};

objid createSound(objid sceneId, std::string soundObjName, std::string clip, bool loop){
  modassert(soundObjName.at(0) == '&', "sound obj must start with &");
  GameobjAttributes attr {
    .attr = {
      { "clip", clip },
      { "center", "true" },
    },
  };
  if (loop){
    attr.attr["loop"] = "true";
  }

  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  auto soundObjId = gameapi -> makeObjectAttr(sceneId, soundObjName, attr, submodelAttributes);
  modassert(soundObjId.has_value(), "sound already exists in scene: " + std::to_string(sceneId));
  return soundObjId.value();
}

// no reason for this to have to create a gameobj to load a texture
void ensureManagedTexturesLoaded(objid id, objid sceneId, std::vector<std::string> textures){
  modlog("ensureManagedTexturesLoaded loaded: ", std::to_string(id));
  modassert(sounds.sceneIdToTextures.find(id) == sounds.sceneIdToTextures.end(), "ensureManagedTexturesLoaded scene id already loaded");
  std::vector<objid> soundIds;
  for (auto &texture : textures){
    GameobjAttributes attr {
      .attr = {
        { "texture", texture },
      },
    };
    attr.attr["texture"] = texture;

    auto soundObjName = std::string("code-texture") + uniqueNameSuffix();
    std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
    auto textureObjId = gameapi -> makeObjectAttr(sceneId, soundObjName, attr, submodelAttributes);
    modassert(textureObjId.has_value(), "obj already exists in scene: " + std::to_string(sceneId));   
    soundIds.push_back(textureObjId.value()); 
  }
  sounds.sceneIdToTextures[id] = soundIds;
}
void unloadManagedTexturesLoaded(objid id){
  modlog("ensureManagedTexturesLoaded try unloaded: ", std::to_string(id));

  if (sounds.sceneIdToTextures.find(id) != sounds.sceneIdToTextures.end()){
    modlog("ensureManagedTexturesLoaded unloaded: ", std::to_string(id));
    auto objIds = sounds.sceneIdToTextures.at(id);
    for (auto soundId : objIds){
      gameapi -> removeByGroupId(soundId);
    }
    sounds.sceneIdToTextures.erase(id);
  }
}

void ensurePrecachedModels(objid sceneId, std::vector<std::string> models){  // obviously inefficient since could just populate the cache directly
  auto oldIds = precachedResources.ids;
  precachedResources.models = models;

  std::vector<objid> newIds;
  for (auto &model : models){
    GameobjAttributes attr {
      .attr = {
        { "mesh", model },
        //{ "disabled", "false" },  why doesn't this work?
        { "position", glm::vec3(10000.f, -10000.f, 10000.f) },
      }
    };
    std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
    auto id = gameapi -> makeObjectAttr(sceneId, std::string("cached-model") + uniqueNameSuffix(), attr, submodelAttributes);
    newIds.push_back(id.value());

    gameapi -> setSingleGameObjectAttr(id.value(), "disabled", "true");  // this doesn't work right now?  just putting position at big
  }
  for (auto id : oldIds){
    gameapi -> removeObjectById(id);
  }
  precachedResources.ids = newIds;
}


