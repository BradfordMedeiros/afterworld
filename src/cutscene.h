#ifndef MOD_AFTERWORLD_CUTSCENE
#define MOD_AFTERWORLD_CUTSCENE

#include "./util.h"
#include "./resources/paths.h"

struct CutsceneApi {
	std::function<void(std::string title, float duration)> showLetterBox;
	std::function<void(std::optional<std::string> camera, glm::vec3 position, glm::quat rotation, std::optional<float> duration)> setCameraPosition;
	std::function<void(bool)> setPlayerControllable;
	std::function<void()> goToNextLevel;
	std::function<void(std::string field, std::string name, AttributeValue)> setWorldState;
};

struct BackgroundFill {
	glm::vec4 color;
	int scheduleId = 0;
};
struct DebugTextDisplay {
	std::string text;
	float rate;
	float size;
	glm::vec2 pos;

	int scheduleId = 0;
};
struct ImageDisplay {
	glm::vec2 pos;
	glm::vec2 size;
	int zIndex = -1;
	glm::vec4 tint = glm::vec4(1.f, 1.f, 1.f, 1.f);
	const char* image = NULL;

	int scheduleId = 0;
};

struct Letterbox {
	std::string text;
};

struct CameraView {
	std::optional<std::string> name;
	glm::vec3 position;
	glm::quat rotation;
	std::optional<float> duration = std::nullopt;
};
struct TerminalDisplay {};

struct ChangePlayable {
	bool isPlayable;
};
struct NextLevel {};

struct WorldState {
	const char* field;
	const char* name;
	AttributeValue value;
};


typedef std::variant<
	ImageDisplay,
	DebugTextDisplay, 
	BackgroundFill, 
	Letterbox, 
	CameraView, 
	TerminalDisplay, 
	ChangePlayable, 
	NextLevel,
	WorldState
> CutsceneEventType;



struct TriggerType {
	std::optional<float> time;
	bool waitForLastEvent;
	std::optional<std::string> waitForMessage;
};

struct CutsceneEvent {
	std::string name;
	float duration;
	TriggerType time;
	CutsceneEventType type;
};

struct Cutscene {
	std::vector<CutsceneEvent> events;
};

struct CutsceneTiming {
	float startTime;
	std::optional<float> endTime;
};
struct CutsceneInstance {
	float startTime;
	bool shouldRemove;
	objid ownerObjId;
	std::set<std::string> messages;
	std::unordered_map<int, CutsceneTiming> playedEvents;
	Cutscene* cutscene;
};

void playCutscene(objid ownerObjId, std::string cutsceneName, float startTime);
void tickCutscenes(CutsceneApi& api, float time);
void onCutsceneMessages(std::string& key);
void onCutsceneObjRemoved(objid id);


/////////////////////////////

struct EasyCutscene {
  std::set<int> playedEvents;
  std::unordered_map<int, float> startTime;
  std::any storage;

  std::vector<int> idsThisFrame;
  std::set<int> playedEventsThisFrame;
  bool firstRun = true;
  bool finished = false;
};

objid playCutscene2(std::function<void(EasyCutscene&)> cutsceneFn);
void removeCutscene(objid id);

void tickCutscenes2();

bool initialize(EasyCutscene& easyCutscene);
void waitUntil(EasyCutscene& easyCutscene, int index, int milliseconds);
void waitFor(EasyCutscene& easyCutscene, int index, std::function<bool()> fn);
void run(EasyCutscene& easyCutscene, int index, std::function<void()> fn);
bool finished(EasyCutscene& easyCutscene, int index);
bool finishedThisFrame(EasyCutscene& easyCutscene, int index);
bool finalize(EasyCutscene& cutscene);

#endif