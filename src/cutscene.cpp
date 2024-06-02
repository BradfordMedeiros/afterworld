#include "./cutscene.h"

extern CustomApiBindings* gameapi;

struct BackgroundFill {
	float duration;
	glm::vec4 color;
};
struct DebugTextDisplay {
	float duration;
	std::string text;
};
struct Letterbox {
	std::string text;
};
struct EmptyEvent {};

typedef std::variant<DebugTextDisplay, BackgroundFill, Letterbox, EmptyEvent> CutsceneEventType;


struct TriggerTime { 
	float time;
};
struct TriggerTimeDuration {
	float time;
	float duration;
};
struct TriggerFromLastEvent {
	float timeOffset;
};
typedef std::variant<TriggerTime, TriggerFromLastEvent> TriggerType;

struct CutsceneEvent {
	std::string name;
	TriggerType time;
	CutsceneEventType type;
};

struct Cutscene {
	std::vector<CutsceneEvent> events;
};

struct CutsceneInstance {
	float startTime;
	int lastPlayedIndex;
	Cutscene* cutscene;
};

std::unordered_map<objid, std::function<void()>> perFrameEvents;


void doCutsceneEvent(CutsceneApi& api, CutsceneEvent& event){
	auto debugTextDisplayPtr = std::get_if<DebugTextDisplay>(&event.type);
	auto backgroundFillPtr = std::get_if<BackgroundFill>(&event.type);
	auto letterboxPtr = std::get_if<Letterbox>(&event.type);
	auto emptyEventPtr = std::get_if<DebugTextDisplay>(&event.type);
	if (debugTextDisplayPtr){
		auto id = getUniqueObjId();
		std::string text = debugTextDisplayPtr -> text;
		perFrameEvents[id] = [text]() -> void {
			drawCenteredText(text, 0.f, 0.f, 0.05f, std::nullopt /* tint */, std::nullopt);
		};
  	gameapi -> schedule(-1, debugTextDisplayPtr -> duration * 1000, NULL, [id](void*) -> void {
  		perFrameEvents.erase(id);
  	});
	}else if (backgroundFillPtr){
		auto id = getUniqueObjId();
		auto color = backgroundFillPtr -> color;
		perFrameEvents[id] = [color]() -> void {
			gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, color, std::nullopt, true, std::nullopt, std::nullopt);
		};
  	gameapi -> schedule(-1, backgroundFillPtr -> duration * 1000, NULL, [id](void*) -> void {
  		perFrameEvents.erase(id);
  	});
	}else if (letterboxPtr){
		api.showLetterBox(letterboxPtr -> text);
	}else if (emptyEventPtr){

	}

	modlog("cutscene", std::string("event - name") + event.name);
}

std::unordered_map<std::string, Cutscene> cutscenes {
	{
		"test", Cutscene {
			.events = {
				CutsceneEvent {
					.name = "initial title text",
					.time = TriggerTime { .time = 0.f },
					.type = DebugTextDisplay {
						.duration = 5.f,
						.text = "Welcome to the Afterworld",
					},
				},
				CutsceneEvent {
					.name = "backgroundfill",
					.time = TriggerTime { .time = 5.f },
					.type = BackgroundFill {
						.duration = 1.f,
						.color = glm::vec4(1.f, 0.f, 0.f, 0.1f),
					},
				},
				CutsceneEvent {
					.name = "second text",
					.time = TriggerTime { .time = 10.f },
					.type = DebugTextDisplay {
						.duration = 15.f,
						.text = "Don't you visit here every day?",
					},
				},
				CutsceneEvent {
					.name = "second text",
					.time = TriggerTime { .time = 10.f },
					.type = Letterbox {
						.text = "and so it starts..."
					},
				},
				CutsceneEvent {
					.name = "ending",
					.time = TriggerTime { .time = 0.f },
					.type = EmptyEvent{},
				},
			},
		} 
	}
};



std::unordered_map<objid, CutsceneInstance> playingCutscenes;

void playCutscene(std::string&& cutsceneName, float startTime){
	modassert(cutscenes.find(cutsceneName) != cutscenes.end(), std::string("play custscene, cutscene does not exist: ") + cutsceneName)

	CutsceneInstance cutsceneInstance { 
		.startTime = startTime, 
		.lastPlayedIndex = -1,
		.cutscene = &cutscenes.at(cutsceneName),
	};
	auto cutsceneId = getUniqueObjId();
	playingCutscenes[cutsceneId] = cutsceneInstance;
	modlog("cutscene", std::string("added cutscene: ") + std::to_string(cutsceneId));
}


std::optional<int> getNextEventIndex(CutsceneInstance& instance, float time, bool* _finished){
	Cutscene& cutscene = *instance.cutscene;
	*_finished = instance.lastPlayedIndex == cutscene.events.size() - 1;
	if (*_finished){
		return std::nullopt;
	}
	for (int i = (instance.lastPlayedIndex + 1); i < cutscene.events.size(); i++){
		auto timeValue = std::get_if<TriggerTime>(&cutscene.events.at(i).time);
		modassert(timeValue, std::string("trigger is not time: ") + cutscene.events.at(i).name);
		if (time >= (instance.startTime + timeValue -> time)) {
			return i;
		}else{
			break;
		}
	}
	return std::nullopt;
}

void tickCutscenes(CutsceneApi& api, float time){
	std::vector<objid> idsToRemove;
	for (auto &[id, instance] : playingCutscenes){
		bool finished = false;
		auto nextEventIndex = getNextEventIndex(instance, time, &finished);
		if (finished){
			idsToRemove.push_back(id);
			continue;
		}
		if (nextEventIndex.has_value()){
			CutsceneEvent& event = instance.cutscene -> events.at(nextEventIndex.value());
			instance.lastPlayedIndex = nextEventIndex.value();
			doCutsceneEvent(api, event);			
		}
	}

	for (auto id : idsToRemove){
		playingCutscenes.erase(id);
		modlog("cutscene", std::string("removed cutscene: ") + std::to_string(id));
	}

	for (auto &[_, fn] : perFrameEvents){
		fn();
	}
}