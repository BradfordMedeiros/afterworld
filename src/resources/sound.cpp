#include "./sound.h"

extern CustomApiBindings* gameapi;

std::string readFileOrPackage(std::string filepath);

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

std::string symbolStrForMixedSound(MixedSound& mixedSound){
  std::string value;
  for (int i = 0; i < mixedSound.soundBinding.folder.size(); i++){
    value += mixedSound.soundBinding.folder.at(i) + "/";
  }
  value += mixedSound.soundBinding.sound;
  return value;
}


MixedSound parsedMixedSound(std::string& filepath){
  auto fileInfo = decomposePath(filepath);
  auto relativeDir = relativePath("../afterworld/data/sounds", fileInfo.dirPath, ".");
  auto relativeDirVec = split(relativeDir, '/');

  float volume = 1.f;
  bool center = false;
  bool loop = false;
  bool sequential = false;
  SoundBus bus = BUS_MASTER;

  auto fileContent = readFileOrPackage(filepath);
  rapidjson::Document doc;
  rapidjson::ParseResult ok = doc.Parse(fileContent.c_str());
  if (doc.HasParseError()){
    std::cout << "error parsing game file: " << filepath << "  (" << fileContent << ")" << std::endl;
  }
  {
    auto it = doc.FindMember("volume");
    if (it != doc.MemberEnd() && it -> value.IsFloat()) {
      volume = it -> value.GetFloat();
    }        
  }
  {
    auto it = doc.FindMember("center");
    if (it != doc.MemberEnd() && it -> value.IsBool()) {
      center = it -> value.GetBool();
    }        
  }
  {
    auto it = doc.FindMember("loop");
    if (it != doc.MemberEnd() && it -> value.IsBool()) {
      loop = it -> value.GetBool();
    }        
  }
  {
    auto it = doc.FindMember("sequential");
    if (it != doc.MemberEnd() && it -> value.IsBool()) {
      sequential = it -> value.GetBool();
    }        
  }

  {
    auto it = doc.FindMember("bus");
    if (it != doc.MemberEnd() && it -> value.IsString()){
      std::string busStr = it -> value.GetString();
      bus = stringToSoundBus(busStr);
    }
  }

  std::vector<std::string> clips;
  {
    auto it = doc.FindMember("clips");
    if (it != doc.MemberEnd() && it->value.IsArray()) {
        for (auto& item : it->value.GetArray()) {
            if (item.IsString()) {
                clips.push_back(item.GetString());
            }
        }
    }
  }


  return MixedSound{
    .clips = clips,
    .volume = volume,
    .center = center,
    .loop = loop,
    .clipOrderSequential = sequential,
    .bus = bus,
    .soundBinding = SoundBinding {
      .filepath = filepath,
      .sound = fileInfo.filename,
      .folder = relativeDirVec,
    },
  };
}

std::vector<MixedSound> createMixedSounds(){
  std::vector<MixedSound> mixedSounds;

  auto mixedSoundFiles = listFilesWithExtensionsFromPackage("../afterworld/data/sounds", { "json" });

  for (auto& mixedSoundFile : mixedSoundFiles){
    mixedSounds.push_back(parsedMixedSound(mixedSoundFile));
  }

  for (auto& mixedSound : mixedSounds){
    auto name = symbolStrForMixedSound(mixedSound);
    int symbol = getSymbol(name);
    mixedSound.nameSymbol = symbol;

  }
  return mixedSounds;
}
std::vector<MixedSound> mixedSounds = createMixedSounds();

int symbolForMixedSound(MixedSound& mixedSound){
  auto name = symbolStrForMixedSound(mixedSound);
  int symbol = getSymbol(name);
  return symbol;
}

SoundInfo getSoundInfo(){
  SoundInfo soundInfo{};
  for(auto& mixedSound : mixedSounds){
    soundInfo.soundBindings.push_back(mixedSound.soundBinding);
  }
  return soundInfo;
}


struct ClipInstance {
  std::string clip;
  objid id;
};

std::vector<ClipInstance> clipInstances;

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

void ensureMixedSoundLoaded(MixedSound& mixedSound, objid sceneId){
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
void ensureMixedSoundsLoaded(objid sceneId){
  for (auto& mixedSound : mixedSounds){
    ensureMixedSoundLoaded(mixedSound, sceneId);
  }
}


void enableMixedSoundClip(MixedSound& mixedSound, int index){
  auto sceneId = gameapi -> rootSceneId();
  for (int i = 0; i <= index; i++){
    if (mixedSound.clips.size() <= i){
      mixedSound.clips.push_back(paths::DEFAULT_SOUND);
    }
  }
  ensureMixedSoundLoaded(mixedSound, sceneId);
}
void disableMixedSoundClip(MixedSound& mixedSound, int index){
  std::vector<std::string> newClips;
  for (int i = 0; i < mixedSound.clips.size(); i++){
    if (i != index){
      newClips.push_back(mixedSound.clips.at(i));
    }
  }

  auto sceneId = gameapi -> rootSceneId();
  mixedSound.clips = newClips;
  ensureMixedSoundLoaded(mixedSound, sceneId);
}
void setMixedSoundClip(MixedSound& mixedSound, std::string clip, int index){
  std::vector<std::string> newClips;
  for (int i = 0; i < mixedSound.clips.size(); i++){
    newClips.push_back((index == i) ? clip : mixedSound.clips.at(i));
  }

  auto sceneId = gameapi -> rootSceneId();
  mixedSound.clips = newClips;
  ensureMixedSoundLoaded(mixedSound, sceneId); 
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

  float volume = mixedSound -> volume; // need to get this from mix
  bool loop = mixedSound -> loop; // same
  bool center = mixedSound -> center; // same

  return gameapi -> playOneshot(clipInstanceId, position, volume, loop, center, clipInstanceId);
}

std::optional<MixedSound*> getMixedSound(std::string name){
  for (auto& mixedSound : mixedSounds){
    if (mixedSound.soundBinding.sound == name){
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

std::optional<std::string> activeMixedSoundStr;
std::optional<std::string> activeMixedSound(){
  return activeMixedSoundStr;
}
void setActiveMixedSound(SoundBinding& soundBinding){
  activeMixedSoundStr = soundBinding.sound;
};

void saveMixedSound(MixedSound& mixedSound){
  rapidjson::Document doc;
  doc.SetObject();
  rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

  doc.AddMember("volume", mixedSound.volume, allocator);
  doc.AddMember("center", mixedSound.center, allocator);
  doc.AddMember("loop", mixedSound.loop, allocator);
  doc.AddMember("sequential", mixedSound.clipOrderSequential, allocator);

  auto bus = soundBusToStr(mixedSound.bus);
  doc.AddMember("bus", bus, allocator);

  rapidjson::Value jsonArray(rapidjson::kArrayType);

  for (auto& clip : mixedSound.clips){
    jsonArray.PushBack(rapidjson::Value(clip, allocator), allocator);
  }
  doc.AddMember("clips", jsonArray, allocator);

  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);

  auto strValue = buffer.GetString();
  std::cout << strValue << std::endl;

  realfiles::saveFile(mixedSound.soundBinding.filepath, strValue);
}