#include "./cutscene.h"

extern CustomApiBindings* gameapi;
extern GLFWwindow* window;

struct CutsceneInstance2 {
	EasyCutscene easyCutscene;
	std::function<void(EasyCutscene&)> cutsceneFn;
	std::optional<objid> ownerId;
};

std::unordered_map<objid, CutsceneInstance2> playingCutscenes2 {};
std::vector<objid> cutscenesToRemove;

objid playCutscene(std::function<void(EasyCutscene&)> cutsceneFn, std::optional<objid> ownerId){
	auto cutsceneId = ownerId.has_value() ? ownerId.value() : getUniqueObjId();
	CutsceneInstance2 cutsceneInstance {
		.easyCutscene = EasyCutscene{
			.cutsceneId = cutsceneId,
		},
		.cutsceneFn = cutsceneFn,
		.ownerId = ownerId,
	};

	playingCutscenes2[cutsceneId] = cutsceneInstance;

	modlog("cutscene", std::string("cutscene added: ") + std::to_string(cutsceneId));
	return cutsceneId;
}

void removeCutscene(objid id, bool forceTick){
	cutscenesToRemove.push_back(id);
	if (forceTick){ // maybe this should just tick this one 
		tickCutscenes2();
	}
}

void tickCutscenes2(){
	for (auto& [cutsceneId, cutscene] : playingCutscenes2){
		cutscene.easyCutscene.idsThisFrame = {};
		cutscene.easyCutscene.playedEventsThisFrame = {};
		cutscene.cutsceneFn(cutscene.easyCutscene);

		bool shouldRemoveThisFrame = false;
		if (cutscene.easyCutscene.idsThisFrame.size() == 0){
			shouldRemoveThisFrame = true;
		}else{
			bool finalIdFinished = true;
			for (auto& id : cutscene.easyCutscene.idsThisFrame){
				if (cutscene.easyCutscene.playedEvents.count(id) == 0){
					finalIdFinished = false;
					break;
				}
			}
			if (finalIdFinished){
				shouldRemoveThisFrame = true;
			}
		}

		if (shouldRemoveThisFrame){
			cutscenesToRemove.push_back(cutsceneId);
		}
		if (!shouldRemoveThisFrame){
			for (auto id : cutscenesToRemove){  // because remove can be called manually
				if (id == cutsceneId){
					shouldRemoveThisFrame = true;
					break;
				}
			}
		}

		if (shouldRemoveThisFrame){
		  cutscene.easyCutscene.finished = true;
			cutscene.easyCutscene.idsThisFrame = {};
			cutscene.easyCutscene.playedEventsThisFrame = {};
		  cutscene.cutsceneFn(cutscene.easyCutscene);
		}
	}
	for (auto id : cutscenesToRemove){
		modlog("cutscene removed", std::to_string(id));
		playingCutscenes2.erase(id);
	}

	cutscenesToRemove = {};
}

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

void waitFor(EasyCutscene& easyCutscene, int index, std::function<bool()> fn){
	easyCutscene.idsThisFrame.push_back(index);

  if (easyCutscene.playedEvents.count(index) > 0){
    return;
  }

  bool success = fn();
  if (success){
  	easyCutscene.playedEvents.insert(index);
  	easyCutscene.playedEventsThisFrame.insert(index);
  }
}

void run(EasyCutscene& easyCutscene, int index, std::function<void()> fn){
	bool dependenciesClear = true;
	for (auto id : easyCutscene.idsThisFrame){
		if (easyCutscene.playedEvents.count(id) == 0){
			dependenciesClear = false;
			break;
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

void store(EasyCutscene& cutscene, std::any data){
	cutscene.storage = data;
}

void setCutsceneFinished(EasyCutscene& cutscene){
	removeCutscene(cutscene.cutsceneId);
}


///////////////

void showLetterBox(std::string title, float duration);
void showLetterBoxHold(std::string title, float fadeInTime);
void hideLetterBox();

void setEntityControlDisabled(bool isDisabled, objid id);
std::optional<objid> getEntityForPlayerIndex(int playerIndex);
int getDefaultPlayerIndex();
void setTempCamera(std::optional<objid> camera, int playerIndex);
void ballModeLevelSelect();

struct OpeningData {
	std::string text;
	float time;
};
void playOpening(EasyCutscene& cutscene){
	static bool didLoadShader = false;
	static unsigned int* shaderId = 0;
	
	if (!didLoadShader){
		shaderId = gameapi -> loadShader("storyboard", paths::SHADER_STORYBOARD);
		didLoadShader = true;
	}
	if(initialize(cutscene)){
		showLetterBox("", 25.f);
		setEntityControlDisabled(true, getEntityForPlayerIndex(0).value());
		store(cutscene, OpeningData{
			.text = "You Have Fallen Asleep.",
			.time = gameapi -> timeSeconds(false),
		});
	}
	if (finalize(cutscene)){
		hideLetterBox();
	}

  OpeningData* value = getStorage<OpeningData>(cutscene);
  modassert(value, "openingData is null");

	waitUntil(cutscene, 0, 10000);
	run(cutscene, 1, [value]() -> void {
		value -> time = gameapi -> timeSeconds(false);
		value -> text = "It is Not Yet Time To Wake Up";
	});

	waitUntil(cutscene, 2, 15000);
	run(cutscene, 3, [value]() -> void {
		value -> time = gameapi -> timeSeconds(false);
		value -> text = "It is Time to [?????]";
	});


	waitUntil(cutscene, 4, 25000);
	if (!finished(cutscene, 4)){
		gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, paths::IMAGE_CUTSCENE_HELL, ShapeOptions {
		 	.shaderId = *shaderId,
		 	//.zIndex = zIndex,
		});

		auto currIndex = static_cast<int>((gameapi -> timeSeconds(false) - value -> time) * 10.f);
		auto textSubtr = value -> text.substr(0, currIndex);
		drawRightText(textSubtr, -0.4f, 0.f, 0.04f, std::nullopt /* tint */, std::nullopt, std::nullopt);
	}else{
		drawRightText("post scene", -0.4f, 0.f, 0.04f, std::nullopt /* tint */, std::nullopt, std::nullopt);
	}

	run(cutscene, 5, []() -> void {
		auto testViewObj = findObjByShortName(">testview", std::nullopt);
		setTempCamera(testViewObj.value(), getDefaultPlayerIndex());     
		gameapi -> moveCameraTo(testViewObj.value(), glm::vec3(0.f, 5.f, 15.f), 0.f);
		gameapi -> setGameObjectRot(testViewObj.value(), parseQuat(glm::vec4(0.f, 0, -1.f, 0.f)), true, Hint { .hint = "cutscene - setCameraPosition" });

		gameapi -> moveCameraTo(testViewObj.value(), glm::vec3(0.f, 5.f, 25.f), 5.f);

	});

	waitUntil(cutscene, 6, 30000);
	run(cutscene, 7, []() -> void {
		setEntityControlDisabled(false, getEntityForPlayerIndex(0).value());
	});
}

void playCutsceneScript(objid ownerObjId, std::string cutsceneName){
	if (cutsceneName == "opening"){
		playCutscene(playOpening, ownerObjId);
		return;
	}
}


///Below here are just default cutscene types, these should be generic probably 
/////////////////



std::unordered_map<std::string, CutsceneOption> cutsceneDatas {
	{ "testorb", CutsceneOption {
			.text = { "I remember a nightmare I had as a child.\n\n"
"A large pyramid\n"
"moving slowly\n"
"on a tilted plane.\n\n"
"There was nothing.\n"
"And yet,\n"
"it terrified me more than anything else." },
			.rail = "cutscene1-rail",
			.letterbox = "Nothing to Be Afraid Of",
	}},
	{ "testorb3", CutsceneOption {
			.text = { "This is the first page of text", "This is second page of text", "This is third page of text" },
			.rail = "cutscene1-rail",
			.letterbox = "Abstract Geometry",
	}},
};
struct CutsceneIntroData {
	bool showText;
	glm::vec3 initialPos;
	objid cameraId;
	std::vector<int> times;
	int railLengthMs;
};

std::function<void(EasyCutscene&)> simpleNarratedMovement(std::string option, std::optional<glm::vec3> position, bool skipAnimation, std::function<void()> onFinish){
	auto& cutsceneData = cutsceneDatas.at(option);
 	auto text = cutsceneData.text;
 	auto rail = cutsceneData.rail;
 	auto letterboxText = cutsceneData.letterbox;

 	bool hasAlreadyPlayed = cutsceneData.hasAlreadyPlayed;
 	cutsceneData.hasAlreadyPlayed = true;

	return [text, rail, letterboxText, position, skipAnimation, hasAlreadyPlayed, onFinish](EasyCutscene& cutscene) -> void {
		int index = -1;
		auto getIndex = [&index]() -> int {
			index++;
			return index;
		};

  	if (initialize(cutscene)){
    	//glm::vec3 initialPos = glm::vec3(0.f, 10.f, 0.f);
    	auto cameraId = findObjByShortName(">menu-view", std::nullopt);
  		auto initialPos = position.has_value() ? position.value() : gameapi -> getGameObjectPos(cameraId.value(), true, "[gamelogic] - ballIntroOpening pos");
    	auto initialRot = gameapi -> getGameObjectRotation(cameraId.value(), true, "[gamelogic] - ballIntroOpening");

  		CutsceneIntroData ballIntroData {
    		.showText = true,
    		.initialPos = initialPos,
    		.cameraId = cameraId.value(),
    		.railLengthMs = 0,
    	};

    	gameapi -> setGameObjectPosition(cameraId.value(), initialPos, true, Hint { .hint = "[gamelogic] - ballIntroOpening" });

			auto railId = railIdForName(rail);

			if (railId.has_value()){
				auto rail = railForId(railId.value());
			 	auto railTotalTimeMs = timeToTriggerIndex(*rail.value(), std::nullopt) * 1000;
			 	std::cout << "cutscene: total length: " << railTotalTimeMs << std::endl;

				std::cout << "cutscene rail: ";
				for (auto& time : rail.value() -> times){
					std::cout << time << " ";
				}
				ballIntroData.times = rail.value() -> times;
				ballIntroData.railLengthMs = railTotalTimeMs;

				std::cout << std::endl;
				addManagedRailMovement(cameraId.value(), railId.value(), initialPos, initialRot);
			}
    	store(cutscene, ballIntroData);
    	showLetterBox(letterboxText, 10.f);
  	}

  	CutsceneIntroData* introData = getStorage<CutsceneIntroData>(cutscene);
  	modassert(introData, "intro data is null");

  	if (finalize(cutscene)){
  		removeManagedRailMovement(introData -> cameraId);
  	  onFinish();
  	}

  	if (skipAnimation || hasAlreadyPlayed){
  		setCutsceneFinished(cutscene);
  	}

  	if (glfwGetKey(window, 'K')){
  		setCutsceneFinished(cutscene);
  	}
  
  	for (int i = 0; i < (text.size() + 1); i++){
  		auto textIndex = getIndex();
  		waitUntil(cutscene, textIndex, i * 5000);
  		if (i < text.size() && finished(cutscene, textIndex)){
	  		gameapi -> drawText(text.at(i), 0.f + 0.1f, i * -0.2f, 12, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  		}
  	}

	};
}

