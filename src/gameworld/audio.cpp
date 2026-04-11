#include "./audio.h"

extern CustomApiBindings* gameapi;

struct AudioClip {
  objid id;
  std::string name;
};

// The default audio clip mechanism is kind of lame, since if you go between and audio zone
// and a default audio zone, it will replay the clip.  Some clips this is fine, but i don't 
// dig restarting it just because it uses a different system than the octree method. 
// I'd prefer this to be just using the tags system and be able to set a default tag value
// if you fall outside of the octree cell. 
std::unordered_map<std::string, AudioClip> audioClips;
std::optional<objid> octreeId;
std::optional<objid> defaultAudioClip;
int loadedDefaultAudioClipFrame = -1;
bool playingDefaultClip = false;
std::optional<std::string> playingClip;
std::string defaultAudioClipPath;

void ensureAllAudioZonesLoaded(int changedLevelFrame, bool hasManagedScene){
  auto mainOctreeId = gameapi -> getMainOctreeId();

  bool changedOctreeState = false;
  if (octreeId != mainOctreeId){
    modlog("octree tags", "unloading");
    for (auto &[_, audioClip] : audioClips){
      gameapi -> removeByGroupId(audioClip.id);
    }
    audioClips = {};    
    octreeId = mainOctreeId;
    changedOctreeState = true;
  }

  if (changedOctreeState){
    modlog("octree tags", "loading");
    auto allTags = gameapi -> getAllTags(getSymbol("audio"));
    for (auto &tag : allTags){
      auto soundObjName = std::string("&code-sound") + uniqueNameSuffix();
      audioClips[tag.value] = AudioClip {
        .id = createSound(gameapi -> listSceneId(octreeId.value()), soundObjName, tag.value, true),
        .name = soundObjName,
      }; 
      modlog("octree tags load sound", tag.value);
    }
  }

  auto loadLevelFrame = changedLevelFrame;
  if (loadedDefaultAudioClipFrame < loadLevelFrame){
    playingDefaultClip = false;
    loadedDefaultAudioClipFrame = loadLevelFrame;
    modlog("octree tags - default audio", "reload");
    if (defaultAudioClip.has_value()){
      gameapi -> removeByGroupId(defaultAudioClip.value());
    }
    auto soundObjName = std::string("&code-sound") + uniqueNameSuffix();

    if (hasManagedScene){
      if (defaultAudioClipPath != ""){
        defaultAudioClip = createSound(gameapi -> rootSceneId(), soundObjName, defaultAudioClipPath, true);      
      }
    }
  }
}

void ensureAmbientSound(glm::vec3 cameraPos, int changedLevelFrame, bool hasManagedScene){
  auto audioSymbol = getSymbol("audio");
  auto tags = gameapi -> getTag(audioSymbol, cameraPos);

  std::optional<std::string> clipToPlay;
  bool inAudioZone = false;
  if (tags.size() > 0){
    inAudioZone = true;
    clipToPlay = tags.at(tags.size() - 1).value;
  }
 

  ensureAllAudioZonesLoaded(changedLevelFrame, hasManagedScene);

  if((!inAudioZone && playingClip.has_value()) || (inAudioZone && playingClip.has_value() && clipToPlay.has_value() && playingClip.value() != clipToPlay.value())){
    modlog("octree tags ensureAmbientSound", "stop clip");
    AudioClip& audioClip = audioClips.at(playingClip.value());
    auto sceneId = gameapi -> listSceneId(octreeId.value());
    gameapi -> stopClip(audioClip.name, sceneId);
    playingClip = std::nullopt;
  }
  if (inAudioZone && !playingClip.has_value()){
    if (playingDefaultClip){
      gameapi -> stopClipById(defaultAudioClip.value());
      playingDefaultClip = false;
    }

    modlog("octree tags ensureAmbientSound play clip", clipToPlay.value());

    modassert(audioClips.find(clipToPlay.value()) != audioClips.end(), "octree tags could not find clip");
    AudioClip& audioClip = audioClips.at(clipToPlay.value());
    playingClip = clipToPlay.value();

    playGameplayClipById(audioClip.id, std::nullopt, std::nullopt, false); 
  }

  if (!inAudioZone && !playingDefaultClip && defaultAudioClip.has_value()){
    playingDefaultClip = true;
    playGameplayClipById(defaultAudioClip.value(), std::nullopt, std::nullopt, true); 
  }

  std::cout << "tags ensure ambient sound: " << inAudioZone << std::endl;
}