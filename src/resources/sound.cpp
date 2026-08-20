#include "./sound.h"

extern CustomApiBindings* gameapi;


std::string* getClipForMaterial(SoundData& sound, std::string& material){
	for (auto &sound : sound.sounds){
		if (material == sound.material){
			return &sound.clip;
		}
	}
	return NULL;
}

void loadAllSounds(std::vector<MaterialToSound>& sounds, objid sceneId){
  GameobjAttributes attr {
    .attr = {},
  };
	std::unordered_map<std::string, GameobjAttributes> submodelAttributes;

	for (auto &sound : sounds){
		attr.attr["clip"] = sound.clip;
		auto id = gameapi -> makeObjectAttr(sceneId, std::string("&material-") + sound.material, attr, submodelAttributes);
	  modassert(id.has_value(), "could not make material");
  }
}

void playMaterialSound(SoundData& sound, int32_t sceneId, glm::vec3 position, std::string& material){
  auto clip = getClipForMaterial(sound, material);
  if (!clip){
    return;
  }
  std::cout << "want to play clip: " << *clip << std::endl;
  playGameplayClip(std::string("&material-" + material), sceneId, std::nullopt, position); // should add playclip position
}

void onCollisionEnterSound(SoundData& sound, int32_t sceneId, int32_t obj1, int32_t obj2, glm::vec3 pos){
  std::string material = "wood";
  auto clip = getClipForMaterial(sound, material);
  if (false && clip){
    playGameplayClip(std::string("&material-" + material), sceneId, std::nullopt, pos);
    auto vel1 = getGameObjectVelocity(obj1);;
    auto vel2 = getGameObjectVelocity(obj2);;
    std::cout << "vel1: " << print(vel1) << ", vel2: " << print(vel2) << std::endl;
  }
}

SoundData createSoundData(objid sceneId){
  SoundData sound;
  auto soundsQuery = gameapi -> compileSqlQuery("select material, walk-sound from materials", {});
  bool validSql = false;
  auto soundsResult = gameapi -> executeSqlQuery(soundsQuery, &validSql);

  std::vector<MaterialToSound> sounds;
  for (auto &soundResult : soundsResult){
    if (soundResult.at(0) == ""){
      continue;
    }
    sounds.push_back(MaterialToSound{
      .material = soundResult.at(0),
      .clip = soundResult.at(1),
    });
  }

  sound.sounds = sounds;
  modassert(validSql, "error executing sql query");
  loadAllSounds(sound.sounds, sceneId);
  return sound;
}


float musicVolume = 1.f;
float gameplayVolume = 1.f;
void setMusicVolume(float volume){
  musicVolume = volume;
}
void setGameplayVolume(float volume){
  gameplayVolume = volume;
}
float getMusicVolume(){
  return musicVolume;
}
float getGameplayVolume(){
  return gameplayVolume;
}


OneShot playMusicClipById(objid id, std::optional<float> volume){
  if (!volume.has_value()){
    volume = 1.f;
  }
  volume = volume.value() * musicVolume;
  return gameapi -> playOneshot(id, std::nullopt, volume, std::nullopt, std::nullopt, id);
}

OneShot playGameplayClip(std::string&& clipName, objid sceneId, std::optional<float> volume, std::optional<glm::vec3> position){
  if (!volume.has_value()){
    volume = 1.f;
  }
  volume = volume.value() * gameplayVolume;

  auto clipId = gameapi -> getClipByName(clipName, sceneId);
  modassert(clipId.has_value(), "playGameplayClip clipName does not exist");
  return gameapi -> playOneshot(clipId.value(), position, volume, false, false, clipId.value());
}

OneShot playGameplayClipById(objid id, std::optional<float> volume, std::optional<glm::vec3> position, bool loop){
  std::cout << "playGameplayClipById: " << loop << std::endl;
  return gameapi -> playOneshot(id, position, volume, loop, false, id);
}

OneShot playGameplayClipByIdCenter(objid id, std::optional<float> volume, bool loop){
  std::cout << "playGameplayClipById: " << loop << std::endl;
  return gameapi -> playOneshot(id, std::nullopt, volume, loop, true, id);
}

/* int getSymbol(std::string name);
std::string nameForSymbol(int symbol); */


std::vector<MixedSound> mixedSounds {
  MixedSound {
    .name = "pistol",
    .nameSymbol = getSymbol("pistol"),
    .clips = { 
      paths::EXPLOSION,
      paths::TELEPORT_SOUND,
    },
  },
};

struct ClipInstance {
  std::string clip;
  objid id;
};

std::vector<ClipInstance> clipInstances;


/*
  GameobjAttributes attr {
    .attr = {},
  };
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;

  for (auto &sound : sounds){
    attr.attr["clip"] = sound.clip;
    auto id = gameapi -> makeObjectAttr(sceneId, std::string("&material-") + sound.material, attr, submodelAttributes);
    modassert(id.has_value(), "could not make material");
    */

objid loadMixedSound(std::string clip, objid sceneId){
  std::cout << "load mixed sound: " << clip << std::endl;
  GameobjAttributes attr {
    .attr = {},
  };
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes; 
  attr.attr["clip"] = clip;
  auto id = gameapi -> makeObjectAttr(sceneId, std::string("&sound-") + uniqueNameSuffix(), attr, submodelAttributes);
  return id.value();
}

std::optional<objid> getClipInstance(std::string& clip){
  for (auto& clipInstance : clipInstances){
    if (clipInstance.clip == clip){
      return clipInstance.id;
    }
  }
  return std::nullopt;
}
void ensureMixedSoundsLoaded(objid sceneId){
  for (auto& mixedSound : mixedSounds){
    for (auto& clip : mixedSound.clips){
      auto existingClipInstance = getClipInstance(clip);
      if (!existingClipInstance.has_value()){
        auto id = loadMixedSound(clip, sceneId);
        clipInstances.push_back(ClipInstance {
           .clip = clip,
           .id = id,
        });
         
      }
    }
  }
}

std::optional<OneShot> playMixedSound(int symbol, std::optional<glm::vec3> position){
  MixedSound* mixedSound = NULL;
  for (auto& mixedSoundValue : mixedSounds){
    if (mixedSoundValue.nameSymbol == symbol){
      mixedSound = &mixedSoundValue;
    }
  }
  modassert(mixedSound != NULL, "mixed sound does not exist");
  if (mixedSound -> clips.size() == 0){
    return std::nullopt;
  }

  int clipToPlay = randomNumber(0, mixedSound -> clips.size() - 1);
  auto clipInstanceIdOpt = getClipInstance(mixedSound -> clips.at(clipToPlay));
  modassert(clipInstanceIdOpt.has_value(), "clip instance does not have a value");

  auto clipInstanceId = clipInstanceIdOpt.value();

  std::optional<glm::vec3> effectivePosition = position; // this needs to check if the mixedsound is position or not
  float volume = mixedSound -> volume; // need to get this from mix
  bool loop = mixedSound -> loop; // same
  bool center = mixedSound -> center; // same

  return gameapi -> playOneshot(clipInstanceId, position, volume, loop, center, clipInstanceId);
}

std::optional<MixedSound*> getMixedSound(std::string name){
  for (auto& mixedSound : mixedSounds){
    if (mixedSound.name == name){
      return &mixedSound;
    }
  }
  return std::nullopt;
}

std::vector<std::string> busNames(){
  return { "master", "sfx", "music", "voice" };
}
SoundBus stringToSoundBus(std::string& value){
  if (value == "master"){
    return BUS_MASTER;
  }
  if (value == "sfx"){
    return BUS_SFX;
  }
  if (value == "music"){
    return BUS_MUSIC;
  }
  if (value == "voice"){
    return BUS_VOICE;
  }
  return BUS_MASTER;
}

std::string soundBusToStr(SoundBus soundBus){
  if (soundBus == BUS_MASTER){
    return "master";
  }
  if (soundBus == BUS_SFX){
    return "sfx";
  }
  if (soundBus == BUS_MUSIC){
    return "music";
  }
  if (soundBus == BUS_VOICE){
    return "voice";
  }
  return "master";
}