#include "./resources.h"

extern CustomApiBindings* gameapi;

ManagedSounds sounds {
  .jumpSoundObjId = std::nullopt,
  .landSoundObjId = std::nullopt,
  .moveSoundObjId = std::nullopt,
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
}