#include "./cutscene.h"

extern CustomApiBindings* gameapi;

struct ShowTitle {
	float duration;
	std::optional<std::string> title;
};
struct BackgroundFill {
	float duration;
	glm::vec4 color;
};
struct DebugTextDisplay {
	float duration;
	std::string text;
};
struct EmptyEvent {};

typedef std::variant<ShowTitle, DebugTextDisplay, BackgroundFill, EmptyEvent> CutsceneEventType;

struct CutsceneEvent {
	std::string name;
	float time;
	CutsceneEventType type;
};

struct Cutscene {
	std::vector<CutsceneEvent> events;
};

struct CutsceneInstance {
	float startTime;
	int lastPlayedIndex;
};

struct CutsceneApi {
	std::function<void(std::string text, float duration)> drawText;
};


std::unordered_map<objid, std::function<void()>> perFrameEvents;


void doCutsceneEvent(CutsceneEvent& event){
	auto showTitlePtr = std::get_if<ShowTitle>(&event.type);
	auto debugTextDisplayPtr = std::get_if<DebugTextDisplay>(&event.type);
	auto backgroundFillPtr = std::get_if<BackgroundFill>(&event.type);
	auto emptyEventPtr = std::get_if<DebugTextDisplay>(&event.type);
	if (showTitlePtr){

	}else if (debugTextDisplayPtr){
		auto id = getUniqueObjId();
		perFrameEvents[id] = []() -> void {
			drawCenteredText("hello", 0.f, 0.f, 0.2f, std::nullopt /* tint */, std::nullopt);
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
	}else if (emptyEventPtr){

	}

	modlog("cutscene", std::string("event - name") + event.name);
}

Cutscene cutscene {
	.events = {
		CutsceneEvent {
			.name = "initial title text",
			.time = 0.f,
			.type = DebugTextDisplay {
				.duration = 5.f,
				.text = "Welcome to the Afterworld",
			},
		},
		CutsceneEvent {
			.name = "backgroundfill",
			.time = 5.f,
			.type = BackgroundFill {
				.duration = 5.f,
				.color = glm::vec4(0.f, 0.f, 0.f, 0.4f),
			},
		},
		CutsceneEvent {
			.name = "second text",
			.time = 10.f,
			.type = DebugTextDisplay {
				.duration = 5.f,
				.text = "Don't you visit here every day?",
			},
		},
		CutsceneEvent {
			.name = "ending",
			.time = 25.f,
			.type = EmptyEvent{},
		},
	},
};

std::unordered_map<objid, CutsceneInstance> playingCutscenes;

void playCutscene(float startTime){
	CutsceneInstance cutsceneInstance { 
		.startTime = startTime, 
		.lastPlayedIndex = -1 
	};
	auto cutsceneId = getUniqueObjId();
	playingCutscenes[cutsceneId] = cutsceneInstance;
	modlog("cutscene", std::string("added cutscene: ") + std::to_string(cutsceneId));
}


std::optional<int> getNextEventIndex(CutsceneInstance& instance, float time, bool* _finished){
	*_finished = instance.lastPlayedIndex == cutscene.events.size() - 1;
	if (*_finished){
		return std::nullopt;
	}
	for (int i = (instance.lastPlayedIndex + 1); i < cutscene.events.size(); i++){
		if (time >= (instance.startTime + cutscene.events.at(i).time)) {
			return i;
		}else{
			break;
		}
	}
	return std::nullopt;
}

void tickCutscenes(float time){
	std::vector<objid> idsToRemove;
	for (auto &[id, instance] : playingCutscenes){
		bool finished = false;
		auto nextEventIndex = getNextEventIndex(instance, time, &finished);
		if (finished){
			idsToRemove.push_back(id);
			continue;
		}
		if (nextEventIndex.has_value()){
			CutsceneEvent& event = cutscene.events.at(nextEventIndex.value());
			instance.lastPlayedIndex = nextEventIndex.value();
			doCutsceneEvent(event);			
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