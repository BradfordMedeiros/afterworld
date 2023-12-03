#include "./hotkeys.h"

extern CustomApiBindings* gameapi;

struct HotkeyToMessage {
	int key;
	std::optional<int> action;
	std::string keyToPublish;
	std::string valueToPublish;
};

std::vector<HotkeyToMessage> hotkeys = {
	HotkeyToMessage {
		.key = 48,  // 0
		.action = 0,
		.keyToPublish = "request-change-gun",
		.valueToPublish = "none",
	},
	HotkeyToMessage {
		.key = 49,  // 1
		.action = 0,
		.keyToPublish = "request-change-gun",
		.valueToPublish = "pistol",
	},
	HotkeyToMessage {
		.key = 50,  // 2 
		.action = 0,
		.keyToPublish = "request-change-gun",
		.valueToPublish = "electrogun",
	},
	HotkeyToMessage {
		.key = 51,  // 3
		.action = 0,
		.keyToPublish = "request-change-gun",
		.valueToPublish = "scrapgun",
	},
};

void handleHotkey(int key, int action){
	for (auto &hotkey : hotkeys){
		if (hotkey.key == key && hotkey.action == action){
			gameapi -> sendNotifyMessage(hotkey.keyToPublish, hotkey.valueToPublish);
		}
	}
}

void spawnProcMesh(objid sceneId){
	std::vector<glm::vec3> faces = {
		{ glm::vec3(0.f, 0.f, 0.f) },
		{ glm::vec3(1.f, 0.f, 0.f) },
		{ glm::vec3(0.f, 1.f, 0.f) },
	};
	std::vector<glm::vec3> points = {
		glm::vec3(0.f, 0.f, 0.f),
	};
	gameapi -> generateMesh(faces, points, "proc-sometest-mesh");


	std::map<std::string, std::string> stringAttributes = { { "mesh", "proc-sometest-mesh" } };

  GameobjAttributes attr {
    .stringAttributes = stringAttributes,
    .numAttributes = {},
    .vecAttr = {  .vec3 = {{ "position", glm::vec3(0.f, 0.f, 0.f) }, { "scale", glm::vec3(1.f, 1.f, 1.f) }},  .vec4 = {}},
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  gameapi -> makeObjectAttr(sceneId, "generatedMesh", attr, submodelAttributes);
}

struct PrintObjDebug {
	std::string objname;
	std::string attribute;
	float interval;
};

std::optional<PrintObjDebug> getPrintObjDebug(std::map<std::string, std::string>& args){
	if (args.find("printdebug") == args.end()){
		return std::nullopt;
	}
	auto values = split(args.at("printdebug"), ':');
	modassert(values.size() == 2, "invalid value for printdebug");
	return PrintObjDebug {
		.objname = values.at(0),
		.attribute = values.at(1),
		.interval = parseFloat(args.at("printrate")) / 1000.f,
	};
}
std::optional<objid> findObjByShortName(std::string name){
  auto allSceneIds = gameapi -> listScenes(std::nullopt);
  for (auto id : allSceneIds){
  	auto objId = gameapi -> getGameObjectByName(name, id, true);
  	if (objId.has_value()){
  		return objId.value();
  	}
  }
	return std::nullopt;
}

CScriptBinding hotkeysBinding(CustomApiBindings& api, const char* name){
	auto binding = createCScriptBinding(name, api);
	auto args = api.getArgs();
	auto printKey = args.find("printkey") != args.end();
	auto printObjDebug = getPrintObjDebug(args);
  binding.onKeyCallback = [printKey](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
   	if (printKey){
   		std::cout << "hotkeysBinding: key = " << key << ", action == " << action << ", scancode = " << scancode << ", mods = " << mods << std::endl;
   	}
   	handleHotkey(key, action);
   	if (key == 75){
   		spawnProcMesh(gameapi -> listSceneId(id));
   	}
  };

  if (printObjDebug.has_value()){
  	binding.onFrame = [printObjDebug](int32_t id, void* data) -> void {
  		static float lastPrintTime = 0;
  		auto currTime = gameapi -> timeSeconds(false);
  		if (currTime - lastPrintTime < printObjDebug.value().interval){
  			return;
  		}
 			lastPrintTime = currTime;
  		auto objid = findObjByShortName(printObjDebug.value().objname);
  		if (objid.has_value()){
			  auto objAttr =  gameapi -> getGameObjectAttr(objid.value());
			  auto attr = getAttr(objAttr, printObjDebug.value().attribute);
			  if (!attr.has_value()){
				  modlog("debug attribute", "no value");
			  }else{
				  modlog("debug attribute", print(attr.value()));
			  }
  		}
  	};
  }

	return binding;
}