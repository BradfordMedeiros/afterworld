#include "./debug.h"

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

bool printKey = false;
CScriptBinding debugBinding(CustomApiBindings& api, const char* name){
	auto binding = createCScriptBinding(name, api);
	auto args = api.getArgs();
	printKey = args.find("printkey") != args.end();
  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
   	if (printKey){
   		std::cout << "debugBinding: key = " << key << ", action == " << action << ", scancode = " << scancode << ", mods = " << mods << std::endl;
   	}
   	handleHotkey(key, action);
  };
	return binding;
}