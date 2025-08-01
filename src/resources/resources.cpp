#include "./resources.h"

extern CustomApiBindings* gameapi;

ManagedSounds sounds {
  .jumpSoundObjId = std::nullopt,
  .landSoundObjId = std::nullopt,
  .moveSoundObjId = std::nullopt,
  .activateSoundObjId = std::nullopt,
  .soundObjId = std::nullopt,
  .explosionSoundObjId = std::nullopt,
  .hitmarkerSoundObjId = std::nullopt,
  .teleportObjId = std::nullopt,
  .sceneIdToSounds = {},
  .sceneIdToTextures = {},
};
PrecachedResources precachedResources {
  .models = {},
  .ids = {},
};

ManagedSounds& getManagedSounds(){
  return sounds;
}

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

std::vector<objid> ensureSoundLoadedBySceneId(objid id, objid sceneId, std::vector<std::string>& soundsToLoad){
  modassert(sounds.sceneIdToSounds.find(id) == sounds.sceneIdToSounds.end(), "ensureSoundLoadedBySceneId scene id already loaded");
  std::vector<objid> soundIds;
  for (std::string& sound: soundsToLoad){
    modlog("ensureSoundLoadedBySceneId loaded: ", sound);
    auto soundObjName = std::string("&code-sound") + uniqueNameSuffix();
    auto soundId = createSound(sceneId, soundObjName, sound, false);
    soundIds.push_back(soundId);
  }
  sounds.sceneIdToSounds[id] = soundIds;
  return soundIds;
}
void unloadManagedSounds(objid id){
  if (sounds.sceneIdToSounds.find(id) != sounds.sceneIdToSounds.end()){
    auto objIds = sounds.sceneIdToSounds.at(id);
    for (auto soundId : objIds){
      gameapi -> removeByGroupId(soundId);
    }
    sounds.sceneIdToSounds.erase(id);
  }
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



void ensureDefaultSoundsLoadced(objid sceneId){
  std::string activateClip = paths::ACTIVATE_SOUND;
  if (activateClip != ""){
    if (sounds.activateSoundObjId.has_value()){
      gameapi -> removeByGroupId(sounds.activateSoundObjId.value());
    }
    sounds.activateSoundObjId = createSound(sceneId, ("&code-activate") + uniqueNameSuffix(), activateClip, false);
  }

  std::string soundClip = paths::WEAPON_SWITCH;
  if (soundClip != ""){
    if (sounds.soundObjId.has_value()){
      gameapi -> removeByGroupId(sounds.soundObjId.value());
    }
    sounds.soundObjId = createSound(sceneId, ("&code-teleport") + uniqueNameSuffix(), soundClip, false);
  }

  std::string explosionClip = paths::EXPLOSION;
  if (explosionClip != ""){
    if (sounds.explosionSoundObjId.has_value()){
      gameapi -> removeByGroupId(sounds.explosionSoundObjId.value());
    }
    sounds.explosionSoundObjId = createSound(sceneId, ("&code-explosion") + uniqueNameSuffix(), explosionClip, false);
  }


  std::string hitmarkerClip = paths::HITMARKER;
  if (hitmarkerClip != ""){
    if (sounds.hitmarkerSoundObjId.has_value()){
      gameapi -> removeByGroupId(sounds.hitmarkerSoundObjId.value());
    }
    sounds.hitmarkerSoundObjId = createSound(sceneId, ("&code-hitmarker") + uniqueNameSuffix(), hitmarkerClip, false);
  }

  std::string teleportClip = paths::TELEPORT_SOUND;
  if (teleportClip != ""){
    if (sounds.teleportObjId.has_value()){
      gameapi -> removeByGroupId(sounds.teleportObjId.value());
    }
    sounds.teleportObjId = createSound(sceneId, ("&code-teleport") + uniqueNameSuffix(), teleportClip, false);
  }


}

void ensureSoundsLoaded(objid sceneId, std::string jumpClip, std::string landClip, std::string moveClip){
  if (jumpClip != ""){
    if (sounds.jumpSoundObjId.has_value()){
      gameapi -> removeByGroupId(sounds.jumpSoundObjId.value());
    }
    sounds.jumpSoundObjId = createSound(sceneId, std::string("&code-movement-jump") + uniqueNameSuffix(), jumpClip, false);    
  }
  if (landClip != ""){
    if (sounds.landSoundObjId.has_value()){
      gameapi -> removeByGroupId(sounds.landSoundObjId.value());
    }
    sounds.landSoundObjId = createSound(sceneId, ("&code-movement-land") + uniqueNameSuffix(), landClip, false);
  }

  if (moveClip != ""){
    if (sounds.moveSoundObjId.has_value()){
      gameapi -> removeByGroupId(sounds.moveSoundObjId.value());
    }
    sounds.moveSoundObjId = createSound(sceneId, ("&code-move") + uniqueNameSuffix(), moveClip, false);
  }
}

void ensureSoundUnloaded(objid sceneId, std::optional<objid>* sound){  // this should just centrally loading into a scene, and then can detect
	if (sound -> has_value()){
		if (gameapi -> gameobjExists(sound -> value())){
			auto objSceneId = gameapi -> listSceneId(sound -> value());
			if (objSceneId == sceneId){
				gameapi -> removeByGroupId(sound -> value());
			}
		}
		sounds.jumpSoundObjId = std::nullopt;
	}
}
void ensureSoundsUnloaded(objid sceneId){  // this should just centrally loading into a scene, and then can detect
	ensureSoundUnloaded(sceneId, &sounds.jumpSoundObjId);
	ensureSoundUnloaded(sceneId, &sounds.landSoundObjId);
	ensureSoundUnloaded(sceneId, &sounds.moveSoundObjId);
  ensureSoundUnloaded(sceneId, &sounds.activateSoundObjId);
  ensureSoundUnloaded(sceneId, &sounds.soundObjId);
  ensureSoundUnloaded(sceneId, &sounds.explosionSoundObjId);
  ensureSoundUnloaded(sceneId, &sounds.hitmarkerSoundObjId);
  ensureSoundUnloaded(sceneId, &sounds.teleportObjId);
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


