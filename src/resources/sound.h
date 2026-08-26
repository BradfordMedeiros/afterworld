#ifndef MOD_AFTERWORLD_SOUNDS
#define MOD_AFTERWORLD_SOUNDS

#include <iostream>
#include <vector>
#include <rapidjson/prettywriter.h>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "./paths.h"

struct MaterialToSound {
	std::string material;
	std::string clip;
};

struct SoundData {
	std::vector<MaterialToSound> sounds;
};

struct MessagePlaySound {
  glm::vec3 position;
  std::string material;
};

SoundData createSoundData(objid sceneId);
void playMaterialSound(SoundData& sound, int32_t sceneId, glm::vec3 position, std::string& material);
void onCollisionEnterSound(SoundData& sound, int32_t sceneId, int32_t obj1, int32_t obj2, glm::vec3 pos);

float* getMusicVolume();
float* getGameplayVolume();

OneShot playMusicClipById(objid id, std::optional<float> volume);
OneShot playGameplayClip(std::string&& clipName, objid sceneId, std::optional<float> volume, std::optional<glm::vec3> position);
OneShot playGameplayClipById(objid id, std::optional<float> volume, std::optional<glm::vec3> position, bool loop);
OneShot playGameplayClipByIdCenter(objid id, std::optional<float> volume, bool loop);

struct SoundBinding {
  std::string filepath;
  std::string sound;
  std::vector<std::string> folder;
};
struct SoundInfo {
  std::vector<SoundBinding> soundBindings;
};  
SoundInfo getSoundInfo();

enum SoundBus { BUS_MASTER, BUS_SFX, BUS_MUSIC, BUS_VOICE };
struct MixedSound {
  int nameSymbol;

  std::vector<std::string> clips;

  float volume = 1.f;
  bool center = false;
  bool loop = false;
  bool clipOrderSequential = true;
  SoundBus bus = BUS_MASTER;

  SoundBinding soundBinding;
};
void ensureMixedSoundsLoaded(objid sceneId);
std::optional<OneShot> playMixedSound(int symbol, std::optional<glm::vec3> position);
std::optional<MixedSound*> getMixedSound(std::string name);
std::vector<std::string> busNames();
SoundBus stringToSoundBus(std::string& value);
std::string soundBusToStr(SoundBus soundBus);

void enableMixedSoundClip(MixedSound& mixedSound, int index);
void disableMixedSoundClip(MixedSound& mixedSound, int index);
void setMixedSoundClip(MixedSound& mixedSound, std::string clip, int index);

std::optional<std::string> activeMixedSound();
void setActiveMixedSound(SoundBinding& soundBinding);

void saveMixedSound(MixedSound& mixedSound);

int symbolForMixedSound(MixedSound& mixedSound);

#endif 