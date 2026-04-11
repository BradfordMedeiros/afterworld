#include "./audio.h"

extern CustomApiBindings* gameapi;
extern AudioZones audiozones;

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


void addAudioZone(objid id){
    //modassert(false, "on add ambient");
    modlog("ambient", std::string("entity added") + gameapi -> getGameObjNameForId(id).value());
    audiozones.audiozoneIds.insert(id);
    std::cout << "audio zones: " << print(audiozones.audiozoneIds) << std::endl;  
}

void removeAudioZone(objid id){
    modlog("ambient", std::string("entity removed") + std::to_string(id));
    audiozones.audiozoneIds.erase(id);
    if (audiozones.currentPlaying.has_value()){
      // clip might have already been removed. 
      gameapi -> stopClip(audiozones.currentPlaying.value().clipToPlay, audiozones.currentPlaying.value().sceneId);
      audiozones.currentPlaying = std::nullopt;
    }
    std::cout << "audio zones: " << print(audiozones.audiozoneIds) << std::endl;
}

void onAudioZoneFrame(){
      // TODO perframe
      // This doesn't need to be per frame, can easily make this on the collision callbacks
      auto transform = gameapi -> getView();
      auto hitObjects = gameapi -> contactTestShape(transform.position, transform.rotation, glm::vec3(1.f, 1.f, 1.f));
      std::set<objid> audioZones;
      for (auto &hitobject : hitObjects){
        if (audiozones.audiozoneIds.count(hitobject.id) > 0){
          audioZones.insert(hitobject.id);
        }
      }

      std::optional<objid> ambientZoneDefault = std::nullopt;
      if (audioZones.size() == 0){
        for (auto id : audiozones.audiozoneIds){
          auto isDefault = getSingleAttr(id, "ambient_default").has_value();
          if (isDefault){
            ambientZoneDefault = id;
            break;
          }
        }
      }
      if (ambientZoneDefault.has_value()){
        audioZones.insert(ambientZoneDefault.value());
      }

      bool stoppedClip = false;
      bool startedClip = false;
      
      if (audiozones.currentPlaying.has_value()){
        if (audioZones.count(audiozones.currentPlaying.value().id) == 0){
          // stop playing clip
          auto currentPlaying = audiozones.currentPlaying.value();
          gameapi -> stopClip(currentPlaying.clipToPlay, currentPlaying.sceneId);
          stoppedClip = true;
          audiozones.currentPlaying = std::nullopt;
        }
      }

      if (audioZones.size() > 0 && !audiozones.currentPlaying.has_value()){
        auto firstId = *(audioZones.begin());
        auto sceneId = gameapi -> listSceneId(firstId);
        auto clipToPlay = getSingleAttr(firstId, "ambient").value();
        audiozones.currentPlaying = CurrentPlayingData { .id = firstId, .sceneId = sceneId, .clipToPlay = clipToPlay };
        playGameplayClip(std::move(clipToPlay), sceneId, std::nullopt, std::nullopt);
        startedClip = true;
      }

      if (stoppedClip || startedClip){
        std::cout << "ambient: started = " << (startedClip ? "true" : "false") << ", stopped = " << (stoppedClip ? "true" : "false") << ", audio zones: " << print(audioZones) << std::endl; 
      }


      // get the view location 
      // contact test as ca 
      //    --  std::vector<HitObject> (*contactTestShape)(glm::vec3 pos, glm::quat orientation, glm::vec3 scale);
      // get the audio zone we are in, if any
      // play that audio zone 
}