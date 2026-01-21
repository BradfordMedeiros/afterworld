#include "./cutscene.h"

extern CustomApiBindings* gameapi;

std::unordered_map<objid, CutsceneInstance> playingCutscenes;     // static-state
std::unordered_map<objid, std::function<void()>> perFrameEvents;  // static-state

void doCutsceneEvent(CutsceneApi& api, CutsceneEvent& event, float time, float duration){
	auto debugTextDisplayPtr = std::get_if<DebugTextDisplay>(&event.type);
	auto imageDisplayPtr = std::get_if<ImageDisplay>(&event.type);
	auto backgroundFillPtr = std::get_if<BackgroundFill>(&event.type);
	auto letterboxPtr = std::get_if<Letterbox>(&event.type);
	auto cameraViewPtr = std::get_if<CameraView>(&event.type);
	auto terminalPtr = std::get_if<TerminalDisplay>(&event.type);
	auto playablePtr = std::get_if<ChangePlayable>(&event.type);
	auto nextLevelPtr = std::get_if<NextLevel>(&event.type);
	auto worldStatePtr = std::get_if<WorldState>(&event.type);

	if (debugTextDisplayPtr){
		auto id = getUniqueObjId();
		std::string text = debugTextDisplayPtr -> text;
		auto pos = debugTextDisplayPtr -> pos;
		auto size = debugTextDisplayPtr -> size;

		perFrameEvents[id] = [text, time, pos, size]() -> void {
			auto currIndex = static_cast<int>((gameapi -> timeSeconds(false) - time) * 100.f);
			auto textSubtr = text.substr(0, currIndex);
			drawRightText(textSubtr, pos.x, pos.y, size, std::nullopt /* tint */, std::nullopt, std::nullopt);
		};
  	gameapi -> schedule(-1, false, duration * 1000, NULL, [id](void*) -> void {
  		perFrameEvents.erase(id);
  	});
  	debugTextDisplayPtr -> scheduleId = id;
	}else if (imageDisplayPtr){
		auto id = getUniqueObjId();
		auto pos = imageDisplayPtr -> pos;
		auto size = imageDisplayPtr -> size;
		auto image = imageDisplayPtr -> image;
		auto zIndex = imageDisplayPtr -> zIndex;
		auto tint = imageDisplayPtr -> tint;
		modassert(image, "image is null");

		static bool didLoadShader = false;
		static unsigned int* shaderId = 0;
		if (!didLoadShader){
			shaderId = gameapi -> loadShader("storyboard", paths::SHADER_STORYBOARD);
			didLoadShader = true;
		}
		imageDisplayPtr -> scheduleId = id;
		perFrameEvents[id] = [time, pos, size, image, zIndex, tint]() -> void {
		  gameapi -> drawRect(pos.x, pos.y, size.x, size.y, false, tint, std::nullopt, true, std::nullopt, image, ShapeOptions {
		  	.shaderId = *shaderId,
		  	.zIndex = zIndex,
		  });
		};
  	gameapi -> schedule(-1, false, duration * 1000, NULL, [id](void*) -> void {
  		perFrameEvents.erase(id);
  	});

	}else if (backgroundFillPtr){
		auto id = getUniqueObjId();
		auto color = backgroundFillPtr -> color;
		perFrameEvents[id] = [color]() -> void {
			gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, color, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
		};
  	gameapi -> schedule(-1, false, duration * 1000, NULL, [id](void*) -> void {
  		perFrameEvents.erase(id);
  	});
  	backgroundFillPtr -> scheduleId = id;
	}else if (letterboxPtr){
		api.showLetterBox(letterboxPtr -> text, duration);
	}else if (cameraViewPtr){
		api.setCameraPosition(cameraViewPtr -> name, cameraViewPtr -> position, cameraViewPtr -> rotation, cameraViewPtr -> duration);
	}else if (terminalPtr){
		auto id = getUniqueObjId();
		std::string text = "hello world";
		perFrameEvents[id] = [text, time]() -> void {
			auto currIndex = static_cast<int>((gameapi -> timeSeconds(false) - time) * 100.f);
			auto textSubtr = text.substr(0, currIndex);
			gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(0.f, 0.f, 0.f, 0.98f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
			drawRightText(textSubtr, -1.f, 0.f, 0.02f, std::nullopt /* tint */, std::nullopt, std::nullopt);
		};
  	gameapi -> schedule(-1, false, duration * 1000, NULL, [id](void*) -> void {
  		perFrameEvents.erase(id);
  	});
	}else if (playablePtr){
		api.setPlayerControllable(playablePtr -> isPlayable);
	}else if (nextLevelPtr){
		api.goToNextLevel();
	}else if (worldStatePtr){
		api.setWorldState(worldStatePtr -> field, worldStatePtr -> name, worldStatePtr -> value);
	}

	modlog("cutscene", std::string("event - ") + event.name + ", time = " + std::to_string(time));
}

std::vector<CutsceneEvent> joinCutscenes(std::vector<std::vector<CutsceneEvent>> cutscenes){
	std::vector<CutsceneEvent> combinedEvents;
	for (auto &cutscene : cutscenes){
		for (auto &event : cutscene){
			combinedEvents.push_back(event);
		}
	}
	return combinedEvents;
}

std::vector<CutsceneEvent> createTextCutscene(const char* background, std::vector<std::string> dialog){
  std::vector<CutsceneEvent> events {
		CutsceneEvent {
			.name = "background",
			.duration = 10.f,
			.time = TriggerType {
				.time = 0.f,
				.waitForLastEvent = false,
				.waitForMessage = std::nullopt,
			},
			.type = BackgroundFill {
				.color = glm::vec4(0.f, 0.f, 1.f, 0.f),
			},
		},
		CutsceneEvent {
			.name = "initial display",
			.duration = 25.f,
			.time = TriggerType {
				.time = 0.f,
				.waitForLastEvent = false,
				.waitForMessage = std::nullopt,
			},
			.type = ImageDisplay {
				.pos = glm::vec2(0.f, 0.f),
				.size = glm::vec2(2.f, 2.f),
				.image = background,
			},
		},
		CutsceneEvent {
			.name = "letterbox",
			.duration = 25.f,
			.time = TriggerType {
				.time = 0.f,
				.waitForLastEvent = false,
				.waitForMessage = std::nullopt,
			},
			.type = Letterbox {
				.text = "",
			},
		},
		CutsceneEvent {
			.name = "initial title text",
			.duration = 10.f,
			.time = TriggerType {
				.time = 0.f,
				.waitForLastEvent = false,
				.waitForMessage = std::nullopt,
			},
			.type = DebugTextDisplay {
				.text = "You Have Fallen Asleep.",
				.rate = 0.5f,
				.size = 0.04f,
				.pos = glm::vec2(-0.4f, 0.f),
			},
		},
		CutsceneEvent {
			.name = "initial title text 2",
			.duration = 5.f,
			.time = TriggerType {
				.time = 0.f,
				.waitForLastEvent = true,
				.waitForMessage = std::nullopt,
			},
			.type = DebugTextDisplay {
				.text = "It is Not Yet Time To Wake Up",
				.rate = 0.5f,
				.size = 0.04f,
				.pos = glm::vec2(-0.4f, 0.f),
			},
		},
		CutsceneEvent {
			.name = "initial title text 2",
			.duration = 10.f,
			.time = TriggerType {
				.time = 0.f,
				.waitForLastEvent = true,
				.waitForMessage = std::nullopt,
			},
			.type = DebugTextDisplay {
				.text = "It is Time to [?????]",
				.rate = 0.5f,
				.size = 0.04f,
				.pos = glm::vec2(-0.4f, 0.f),
			},
		},
	};
	/* another clip
		Something happened, I dont know what happened
		but I know somethign happened
		it changed
		but i dont know what the difference is
	*/
  return events;
}

std::unordered_map<std::string, Cutscene> cutscenes {
	{ "opening", Cutscene {
			.events = joinCutscenes({
				{
					CutsceneEvent {
						.name = "stop-playing",
						.duration = 0.f,
						.time = TriggerType {
							.time = 0.f,
							.waitForLastEvent = false,
							.waitForMessage = std::nullopt,
						},
						.type = ChangePlayable { .isPlayable = false },
					},
				},
				createTextCutscene(paths::IMAGE_CUTSCENE_HELL, {}), 
				{
					CutsceneEvent {
						.name = "camera-view-1",
						.duration = 2.f,
						.time = TriggerType {
							.time = 0.f,
							.waitForLastEvent = true,
							.waitForMessage = std::nullopt,
						},
						.type = CameraView {
							.name = ">testview",
							.position = glm::vec3(0.f, 5.f, 15.f),
							.rotation = parseQuat(glm::vec4(0.f, 0, -1.f, 0.f)),
							.duration = std::nullopt,
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
							.name = ">testview",
							.position = glm::vec3(0.f, 5.f, 50.f),
							.rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)),
							.duration = 5.f,
						},
					},
					CutsceneEvent {
						.name = "set sky color",
						.duration = 5.f,
						.time = TriggerType {
							.time = 0.f,
							.waitForLastEvent = true,
							.waitForMessage = std::nullopt,
						},
						.type = WorldState {
							.field = "skybox",
							.name = "color",
							.value = glm::vec3(1.f, 0.f, 0.f),
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
							.name = std::nullopt,
							.position = glm::vec3(0.f, 5.f, 50.f),
							.rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)),
							.duration = 5.f,
						},
					},
					CutsceneEvent {
						.name = "start-playing",
						.duration = 10.f,
						.time = TriggerType {
							.time = 0.f,
							.waitForLastEvent = true,
							.waitForMessage = std::nullopt,
						},
						.type = ChangePlayable { .isPlayable = true },
					},
					//CutsceneEvent {
					//	.name = "next-level",
					//	.duration = 0.f,
					//	.time = TriggerType {
					//		.time = 0.f,
					//		.waitForLastEvent = true,
					//		.waitForMessage = std::nullopt,
					//	},
					//	.type = NextLevel{},
					//},
				}}
			),
		}
	},
};


void playCutscene(objid ownerObjId, std::string cutsceneName, float startTime){
	modassert(cutscenes.find(cutsceneName) != cutscenes.end(), std::string("play custscene, cutscene does not exist: ") + cutsceneName)

	CutsceneInstance cutsceneInstance { 
		.startTime = startTime,
		.shouldRemove = false,
		.ownerObjId = ownerObjId,
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

void finalizeCutscene(CutsceneApi& api, CutsceneInstance& cutsceneInstance, float time){
	std::cout << "finalize cutscene called" << std::endl;
	for (auto &[id, timing] : cutsceneInstance.playedEvents){
		CutsceneEvent& event = cutsceneInstance.cutscene -> events.at(id);
		auto name = event.name;

		if (!timing.endTime.has_value() || timing.endTime.value() < time){
			std::cout << "finalize cutscene - event not finished: " << name << std::endl;
		}

		auto debugTextDisplayPtr = std::get_if<DebugTextDisplay>(&event.type);
		auto imageDisplayPtr = std::get_if<ImageDisplay>(&event.type);
		auto backgroundFillPtr = std::get_if<BackgroundFill>(&event.type);
		auto letterboxPtr = std::get_if<Letterbox>(&event.type);
		auto cameraViewPtr = std::get_if<CameraView>(&event.type);
		auto terminalPtr = std::get_if<TerminalDisplay>(&event.type);
		auto playablePtr = std::get_if<ChangePlayable>(&event.type);
		auto nextLevelPtr = std::get_if<NextLevel>(&event.type);
		auto worldStatePtr = std::get_if<WorldState>(&event.type);

		if (debugTextDisplayPtr){
			perFrameEvents.erase(debugTextDisplayPtr -> scheduleId);
		}else if (imageDisplayPtr){
			perFrameEvents.erase(imageDisplayPtr -> scheduleId);
		}else if (backgroundFillPtr){
			perFrameEvents.erase(backgroundFillPtr -> scheduleId);
		}else if (letterboxPtr){
			api.showLetterBox("", 0.f);
		}else if (cameraViewPtr){

		}else if (terminalPtr){

		}else if (playablePtr){

		}else if (nextLevelPtr){

		}else if (worldStatePtr){

		}else{
			modassert(false, "invalid type cannot finalize");
		}


	}
}

void tickCutscenes(CutsceneApi& api, float time){
	std::vector<objid> idsToRemove;
	for (auto &[id, instance] : playingCutscenes){
		bool finished = false;
		auto nextEventIndex = getNextEventIndex(instance, time, &finished);
		if (finished || instance.shouldRemove){
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
		finalizeCutscene(api, playingCutscenes.at(id), time);
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

void onCutsceneObjRemoved(objid id){
	for (auto &[_, instance] : playingCutscenes){
		if (instance.ownerObjId == id){
			instance.shouldRemove = true;
		}		
	}
}

//////////////////////////
struct EasyCutscene {
  std::optional<float> initialTime = gameapi -> timeSeconds(false);
  std::set<int> playedEvents;
  std::unordered_map<int, float> startTime;
  std::any storage;

  std::vector<int> idsThisFrame;
  std::set<int> playedEventsThisFrame;
  bool firstRun = true;
  bool finished = false;
};

bool initialize(EasyCutscene& easyCutscene){
	bool firstRun = easyCutscene.firstRun;
	if (firstRun){
		easyCutscene.firstRun = false;
	}
  return firstRun;
}

void waitUntil(EasyCutscene& easyCutscene, int index, int milliseconds){
	easyCutscene.idsThisFrame.push_back(index);

  if (easyCutscene.playedEvents.count(index) > 0){
    return;
  }

  if (easyCutscene.startTime.find(index) == easyCutscene.startTime.end()){
	  easyCutscene.startTime[index] = gameapi -> timeSeconds(false);
  }
  auto timeElapsed = gameapi -> timeSeconds(false) - easyCutscene.startTime.at(index);
  if (timeElapsed > (milliseconds / 1000.f)){
  	easyCutscene.playedEvents.insert(index);
  	easyCutscene.playedEventsThisFrame.insert(index);
  }
}

void run(EasyCutscene& easyCutscene, int index, std::function<void()> fn){
	bool dependenciesClear = false;
	if (easyCutscene.idsThisFrame.size() == 0){
		dependenciesClear = true;
	}else{
		auto index = easyCutscene.idsThisFrame.at(easyCutscene.idsThisFrame.size() - 1);
		if (easyCutscene.playedEvents.count(index) > 0){
			dependenciesClear = true;
		}
	}	

	easyCutscene.idsThisFrame.push_back(index);

	if (dependenciesClear && easyCutscene.playedEvents.count(index) == 0){
		fn();
		easyCutscene.playedEvents.insert(index);
		easyCutscene.playedEventsThisFrame.insert(index);
	}
}

bool finished(EasyCutscene& easyCutscene, int index){
  if (easyCutscene.playedEvents.count(index) > 0){
    return true;
  }
  return false;
}

bool finishedThisFrame(EasyCutscene& easyCutscene, int index){
	return easyCutscene.playedEventsThisFrame.count(index) > 0;
}

bool finalize(EasyCutscene& cutscene){
	return cutscene.finished;
}


void playTestCutscene(EasyCutscene& easyCutscene){
  if (initialize(easyCutscene)){
    // ponteitally get initial object positions and shove into std any whatever
  }
  waitUntil(easyCutscene, 0, 1000);
  waitUntil(easyCutscene, 1, 2000);
  run(easyCutscene, 2, []() -> void {
    std::cout << "hello world" << std::endl;
  });

  if (finished(easyCutscene, 1)){

  }
  std::cout << "wow" << std::endl;
}



struct CutsceneInstance2 {
	objid ownerObjId;
	EasyCutscene easyCutscene;
	std::function<void(EasyCutscene&)> cutsceneFn;
};


void testCutscene2(EasyCutscene& cutscene){
	auto showRed = finished(cutscene, 0);
	auto showGreen = finished(cutscene, 1);

	if (initialize(cutscene)){
		std::cout << "testCutscene2 initial" << std::endl;
	}
	if (finalize(cutscene)){
		std::cout << "testCutscene2 finalize" << std::endl;
	}

	waitUntil(cutscene, 0, 5000);
	waitUntil(cutscene, 1, 10000);
	waitUntil(cutscene, 2, 15000);

	if (finishedThisFrame(cutscene, 0)){
		std::cout << "testCutscene2 event 0 finished" << std::endl;
	}
	if (finishedThisFrame(cutscene, 1)){
		std::cout << "testCutscene2 event 1 finished" << std::endl;
	}
	if (finishedThisFrame(cutscene, 2)){
		std::cout << "testCutscene2 event 2 finished" << std::endl;
	}
	
	glm::vec4 color = glm::vec4(0.f, 0.f, 1.f, 1.f);
	if (showRed){
		color = glm::vec4(1.f, 0.f, 0.f, 1.f);
	}
	if (showGreen){
		color = glm::vec4(0.f, 1.f, 0.f, 1.f);
	}
  gameapi -> drawRect(0.5f, 0.f, 1.f, 2.f, false, color, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
}

std::unordered_map<objid, CutsceneInstance2> playingCutscenes2 {};
std::unordered_map<std::string, std::function<void(EasyCutscene&)>> cutscenes2 {
	{ "test", testCutscene2 },
};

void playCutscene2(objid ownerObjId, std::string cutsceneName){
	modassert(cutscenes2.find(cutsceneName) != cutscenes2.end(), std::string("play custscene, cutscene does not exist: ") + cutsceneName)

	CutsceneInstance2 cutsceneInstance {
		.ownerObjId = ownerObjId,
		.cutsceneFn = cutscenes2.at(cutsceneName),
	};

	auto cutsceneId = getUniqueObjId();
	playingCutscenes2[cutsceneId] = cutsceneInstance;

	modlog("cutscene", std::string("added cutscene2: ") + std::to_string(cutsceneId));
}

void onCutsceneObjRemoved2(objid id){

}

void tickCutscenes2(){
	std::vector<objid> cutscenesToRemove;
	for (auto& [cutsceneId, cutscene] : playingCutscenes2){
		cutscene.easyCutscene.idsThisFrame = {};
		cutscene.easyCutscene.playedEventsThisFrame = {};
		cutscene.cutsceneFn(cutscene.easyCutscene);

		bool shouldFinalize = false;
		if (cutscene.easyCutscene.idsThisFrame.size() == 0){
			shouldFinalize = true;
		}else{
			auto lastEventId = cutscene.easyCutscene.idsThisFrame.at(cutscene.easyCutscene.idsThisFrame.size() - 1);
			auto finalIdFinished = cutscene.easyCutscene.playedEvents.count(lastEventId) > 0;
			if (finalIdFinished){
				shouldFinalize = true;
			}
		}

		if (shouldFinalize){
		  cutscene.easyCutscene.finished = true;
		  cutscene.cutsceneFn(cutscene.easyCutscene);
		  cutscenesToRemove.push_back(cutsceneId);
		}
	}
	for (auto id : cutscenesToRemove){
		playingCutscenes2.erase(id);
	}
}

