#ifndef MOD_AFTERWORLD_CUTSCENE
#define MOD_AFTERWORLD_CUTSCENE

#include "../util.h"
#include "../resources/paths.h"
#include "../gameworld/curves.h"  // i dont like this dependency

struct EasyCutscene {
  std::set<int> playedEvents;
  std::unordered_map<int, float> startTime;
  std::any storage;

  std::vector<int> idsThisFrame;
  std::set<int> playedEventsThisFrame;
  bool firstRun = true;
  bool finished = false;
  objid cutsceneId;
};

objid playCutscene(std::function<void(EasyCutscene&)> cutsceneFn, std::optional<objid> ownerId);
void removeCutscene(objid id, bool forceTick = false);

void tickCutscenes2();

bool initialize(EasyCutscene& easyCutscene);
void waitUntil(EasyCutscene& easyCutscene, int index, int milliseconds);
void waitFor(EasyCutscene& easyCutscene, int index, std::function<bool()> fn);
void run(EasyCutscene& easyCutscene, int index, std::function<void()> fn);
bool finished(EasyCutscene& easyCutscene, int index);
bool finishedThisFrame(EasyCutscene& easyCutscene, int index);
bool finalize(EasyCutscene& cutscene);
void store(EasyCutscene& cutscene, std::any data);
void setCutsceneFinished(EasyCutscene& cutscene);

template <typename T>
T* getStorage(EasyCutscene& cutscene){
  try {
    T* value = std::any_cast<T>(&cutscene.storage);
    return value;
  }catch(...){
    return NULL;
  }
}

void playCutsceneScript(objid ownerObjId, std::string cutsceneName);


struct SimpleNarration {
  std::string text;
};
struct NarratedMovement {
  objid railId;
  std::optional<std::string> letterbox;
  std::vector<SimpleNarration> narrations;
};

std::function<void(EasyCutscene&)> simpleNarratedMovement2(objid cameraId, objid sceneId, NarratedMovement cutsceneData, bool skipAnimation, std::function<void()> onFinish);

#endif