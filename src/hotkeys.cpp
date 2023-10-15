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

bool printKey = false;
CScriptBinding hotkeysBinding(CustomApiBindings& api, const char* name){
	auto binding = createCScriptBinding(name, api);
	auto args = api.getArgs();
	printKey = args.find("printkey") != args.end();
  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
   	if (printKey){
   		std::cout << "hotkeysBinding: key = " << key << ", action == " << action << ", scancode = " << scancode << ", mods = " << mods << std::endl;
   	}
   	handleHotkey(key, action);
   	if (key == 75){
   		spawnProcMesh(gameapi -> listSceneId(id));
   	}
  };
	return binding;
}