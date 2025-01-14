#include "./resources.h"

extern CustomApiBindings* gameapi;

ManagedSounds sounds {
  .jumpSoundObjId = std::nullopt,
  .landSoundObjId = std::nullopt,
  .moveSoundObjId = std::nullopt,
  .activateSoundObjId = std::nullopt,
  .soundObjId = std::nullopt,
  .explosionSoundObjId = std::nullopt,
};
PrecachedResources precachedResources {
  .models = {},
  .ids = {},
};

ManagedSounds& getManagedSounds(){
  return sounds;
}

objid createSound(objid sceneId, std::string soundObjName, std::string clip){
  modassert(soundObjName.at(0) == '&', "sound obj must start with &");
  GameobjAttributes attr {
    .attr = {
      { "clip", clip },
      { "center", "true" },
    },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  auto soundObjId = gameapi -> makeObjectAttr(sceneId, soundObjName, attr, submodelAttributes);
  modassert(soundObjId.has_value(), "sound already exists in scene: " + std::to_string(sceneId));
  return soundObjId.value();
}

void ensureDefaultSoundsLoadced(objid sceneId){
  std::string activateClip = "../gameresources/sound/click.wav";
  if (activateClip != ""){
    if (sounds.activateSoundObjId.has_value()){
      gameapi -> removeByGroupId(sounds.activateSoundObjId.value());
    }
    sounds.activateSoundObjId = createSound(sceneId, ("&code-activate") + uniqueNameSuffix(), activateClip);
  }

  std::string teleportClip = "../gameresources/sound/teleport.wav";
  if (teleportClip != ""){
    if (sounds.soundObjId.has_value()){
      gameapi -> removeByGroupId(sounds.soundObjId.value());
    }
    sounds.soundObjId = createSound(sceneId, ("&code-teleport") + uniqueNameSuffix(), teleportClip);
  }

  std::string explosionClip = "../ModEngine/res/sounds/silenced-gunshot.wav";
  if (explosionClip != ""){
    if (sounds.explosionSoundObjId.has_value()){
      gameapi -> removeByGroupId(sounds.explosionSoundObjId.value());
    }
    sounds.explosionSoundObjId = createSound(sceneId, ("&code-explosion") + uniqueNameSuffix(), explosionClip);
  }
}

void ensureSoundsLoaded(objid sceneId, std::string jumpClip, std::string landClip, std::string moveClip){
  if (jumpClip != ""){
    if (sounds.jumpSoundObjId.has_value()){
      gameapi -> removeByGroupId(sounds.jumpSoundObjId.value());
    }
    sounds.jumpSoundObjId = createSound(sceneId, std::string("&code-movement-jump") + uniqueNameSuffix(), jumpClip);    
  }
  if (landClip != ""){
    if (sounds.landSoundObjId.has_value()){
      gameapi -> removeByGroupId(sounds.landSoundObjId.value());
    }
    sounds.landSoundObjId = createSound(sceneId, ("&code-movement-land") + uniqueNameSuffix(), landClip);
  }

  if (moveClip != ""){
    if (sounds.moveSoundObjId.has_value()){
      gameapi -> removeByGroupId(sounds.moveSoundObjId.value());
    }
    sounds.moveSoundObjId = createSound(sceneId, ("&code-move") + uniqueNameSuffix(), moveClip);
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
    std::map<std::string, GameobjAttributes> submodelAttributes;
    auto id = gameapi -> makeObjectAttr(sceneId, std::string("cached-model") + uniqueNameSuffix(), attr, submodelAttributes);
    newIds.push_back(id.value());

    gameapi -> setSingleGameObjectAttr(id.value(), "disabled", "true");  // this doesn't work right now?  just putting position at big
  }
  for (auto id : oldIds){
    gameapi -> removeObjectById(id);
  }
  precachedResources.ids = newIds;
}


