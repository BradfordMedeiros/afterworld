#include "./cutscene.h"

extern CustomApiBindings* gameapi;

struct BackgroundFill {
	float duration;
	glm::vec4 color;
};
struct DebugTextDisplay {
	float duration;
	std::string text;
	float rate;
};
struct Letterbox {
	std::string text;
};

struct CameraView {
	glm::vec3 position;
	glm::quat rotation;
};
struct PopCameraView {};

typedef std::variant<DebugTextDisplay, BackgroundFill, Letterbox, CameraView, PopCameraView> CutsceneEventType;



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
	std::set<std::string> messages;
	std::unordered_map<int, CutsceneTiming> playedEvents;
	Cutscene* cutscene;
};

std::unordered_map<objid, std::function<void()>> perFrameEvents;


void doCutsceneEvent(CutsceneApi& api, CutsceneEvent& event, float time, float duration){
	auto debugTextDisplayPtr = std::get_if<DebugTextDisplay>(&event.type);
	auto backgroundFillPtr = std::get_if<BackgroundFill>(&event.type);
	auto letterboxPtr = std::get_if<Letterbox>(&event.type);
	auto cameraViewPtr = std::get_if<CameraView>(&event.type);
	auto popcameraPtr = std::get_if<PopCameraView>(&event.type);
	if (debugTextDisplayPtr){
		auto id = getUniqueObjId();
		std::string text = debugTextDisplayPtr -> text;

		perFrameEvents[id] = [text, time]() -> void {
			auto currIndex = static_cast<int>((gameapi -> timeSeconds(true) - time) * 100.f);
			auto textSubtr = text.substr(0, currIndex);
			drawRightText(textSubtr, 0.f, 0.f, 0.05f, std::nullopt /* tint */, std::nullopt);
		};
  	gameapi -> schedule(-1, duration * 1000, NULL, [id](void*) -> void {
  		perFrameEvents.erase(id);
  	});
	}else if (backgroundFillPtr){
		auto id = getUniqueObjId();
		auto color = backgroundFillPtr -> color;
		perFrameEvents[id] = [color]() -> void {
			gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, color, std::nullopt, true, std::nullopt, std::nullopt);
		};
  	gameapi -> schedule(-1, duration * 1000, NULL, [id](void*) -> void {
  		perFrameEvents.erase(id);
  	});
	}else if (letterboxPtr){
		api.showLetterBox(letterboxPtr -> text, duration);
	}else if (cameraViewPtr){
		api.setCameraPosition(cameraViewPtr -> position, cameraViewPtr -> rotation);
	}else if (popcameraPtr){
		api.popTempViewpoint();
	}

	modlog("cutscene", std::string("event - name") + event.name + ", time = " + std::to_string(time));
}

std::unordered_map<std::string, Cutscene> cutscenes {
	{
		"test", Cutscene {
			.events = {
				CutsceneEvent {
					.name = "initial title text",
					.duration = 5.f,
					.time = TriggerType {
						.time = 0.f,
						.waitForLastEvent = false,
						.waitForMessage = std::nullopt,
					},
					.type = DebugTextDisplay {
						.text = "Welcome to the Afterworld",
						.rate = 100.f,
					},
				},
				CutsceneEvent {
					.name = "backgroundfill red",
					.duration = 2.f,
					.time = TriggerType {
						.time = 0.f,
						.waitForLastEvent = true,
						.waitForMessage = std::nullopt,
					},
					.type = BackgroundFill {
						.color = glm::vec4(0.f, 0.f, 0.f, 0.5f),
					},
				},
				/*CutsceneEvent {
					.name = "camera-view-1",
					.duration = 5.f,
					.time = TriggerType {
						.time = 0.f,
						.waitForLastEvent = true,
						.waitForMessage = std::nullopt,
					},
					.type = CameraView {
						.position = glm::vec3(0.f, 10.f, 10.f),
						.rotation = parseQuat(glm::vec4(0.f, -1.f, -1.f, 0.f)),
					},
				},
				CutsceneEvent {
					.name = "camera-view-2",
					.duration = 5.f,
					.time = TriggerType {
						.time = 0.f,
						.waitForLastEvent = true,
						.waitForMessage = std::nullopt,
					},
					.type = CameraView {
						.position = glm::vec3(3.f, 10.f, 10.f),
						.rotation = parseQuat(glm::vec4(-1.f, -1.f, 0.f, 0.f)),
					},
				},
				CutsceneEvent {
					.name = "camera-view-3",
					.duration = 5.f,
					.time = TriggerType {
						.time = 0.f,
						.waitForLastEvent = true,
						.waitForMessage = std::nullopt,
					},
					.type = PopCameraView{},
				},*/
				CutsceneEvent {
					.name = "letterbox",
					.duration = 2.f,
					.time = TriggerType {
						.time = 0.f,
						.waitForLastEvent = true,
						.waitForMessage = std::nullopt,
					},
					.type = Letterbox {
						.text = "letter box hello",
					},
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
		.messages = {},
		.playedEvents = {},
		.cutscene = &cutscenes.at(cutsceneName),
	};
	auto cutsceneId = getUniqueObjId();
	playingCutscenes[cutsceneId] = cutsceneInstance;
	modlog("cutscene", std::string("added cutscene: ") + std::to_string(cutsceneId));
}


struct NextEvent {
	int index;
	TriggerType* trigger;
};

bool cutscenesFinished(CutsceneInstance& instance, float time){
	if (instance.playedEvents.size() != instance.cutscene -> events.size()){
		return false;
	}
	for (auto &[_, timing] : instance.playedEvents){
		if (!timing.endTime.has_value()){
			return false;
		}
		if (timing.endTime.value() > time){
			return false;
		}
	}
	return true;
}


bool passesWaitFor(CutsceneInstance& instance, TriggerType& triggerType, int index, float time){
	if (!triggerType.waitForLastEvent){
		return true;
	}
	auto lastIndex = index - 1;
	if (lastIndex < 0){
		return true;
	}
	bool eventAlreadyPlayed = instance.playedEvents.find(lastIndex) != instance.playedEvents.end();
	if (eventAlreadyPlayed){
		CutsceneTiming& timing = instance.playedEvents.at(lastIndex);
		if (timing.endTime.has_value() && time >= timing.endTime.value()){
			return true;
		} 
	}
	return false;
}
bool passesWaitForMessage(CutsceneInstance& instance, TriggerType& triggerType, int index){
	CutsceneEvent& event = instance.cutscene -> events.at(index);
	if (!event.time.waitForMessage.has_value()){
		return true;
	}
	return instance.messages.find(event.time.waitForMessage.value()) != instance.messages.end();
}

std::optional<NextEvent> getNextEventIndex(CutsceneInstance& instance, float time, bool* _finished){
	Cutscene& cutscene = *instance.cutscene;
	*_finished = instance.playedEvents.size() == cutscene.events.size();
	if (*_finished){
		return std::nullopt;
	}

	*_finished = cutscenesFinished(instance, time);
	if (*_finished ){
		return std::nullopt;
	}

	for (int i = 0; i < cutscene.events.size(); i++){
		bool eventAlreadyPlayed = instance.playedEvents.find(i) != instance.playedEvents.end();
		if (eventAlreadyPlayed){
			continue;
		}

		TriggerType& triggerType = cutscene.events.at(i).time;

		bool passesTime = !triggerType.time.has_value() || time >= (instance.startTime + triggerType.time.value());
		bool passesWait = passesWaitFor(instance, triggerType, i, time);
		bool passesWaitMessage = passesWaitForMessage(instance, triggerType, i);
		if (passesTime && passesWait && passesWaitMessage){
			return NextEvent {
				.index = i,
				.trigger = &cutscene.events.at(i).time,
			};
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
			auto index = nextEventIndex.value().index;
			CutsceneEvent& event = instance.cutscene -> events.at(index);
			doCutsceneEvent(api, event, time, event.duration);
			instance.playedEvents[index] = CutsceneTiming {
				.startTime = time,
				.endTime = time + event.duration,
			};
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

void onCutsceneMessages(std::string& key){
	for (auto &[_, instance] : playingCutscenes){
		for (int i = 0; i < instance.cutscene -> events.size(); i++){
//			if (event.time.waitForMessage.has_value() && event.time.waitForMessage.value() == key){
			if (instance.cutscene -> events.at(i).time.waitForMessage.has_value() && instance.cutscene -> events.at(i).time.waitForMessage.value() == key){
				instance.messages.insert(key);
			}
		}
	}
}