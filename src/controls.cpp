#include "./controls.h"

extern CustomApiBindings* gameapi;

int jumpKey = 32;
int moveFowardKey = 'W';
int moveBackwardKey = 'S';
int moveLeftKey = 'A';
int moveRightKey = 'D';
int crouchKey = 341;
int interactKey = 'E';

int fireButton = 0;
int aimButton = 1;

void setControl(int controlSymbol, int key){
	static int jumpSymbol = getSymbol("control-jump");
	static int moveForwardSymbol = getSymbol("control-move-forward");
	static int moveBackwardSymbol = getSymbol("control-move-backward");
	static int moveLeftSymbol = getSymbol("control-move-left");
	static int moveRightSymbol = getSymbol("control-move-right");
	static int crouchSymbol = getSymbol("control-crouch");
	static int interactSymbol = getSymbol("control-interact");

	static int fireSymbol = getSymbol("control-fire");
	static int aimSymbol = getSymbol("control-aim");

	if (controlSymbol == jumpSymbol){
		jumpKey = key;
	}else if (controlSymbol == moveForwardSymbol){
		moveFowardKey = key;
	}else if (controlSymbol == moveBackwardSymbol){
		moveBackwardKey = key;
	}else if (controlSymbol == moveLeftSymbol){
		moveLeftKey = key;
	}else if (controlSymbol == moveRightSymbol){
		moveRightKey = key;
	}else if (controlSymbol == crouchSymbol){
		crouchKey = key;
	}else if (controlSymbol == interactSymbol){
		interactKey = key;
	}else if (controlSymbol == fireSymbol){
		fireButton = key;
	}else if (controlSymbol == aimSymbol){
		aimButton = key;
	}else {
		modassert(false, std::string("invalid control symbol: ") + nameForSymbol(controlSymbol));
	}
}

bool isJumpKey(int key){
	return key == jumpKey; // space
}
bool isMoveForwardKey(int key){
	return key == moveFowardKey;
}
bool isMoveBackwardKey(int key){
	return key == moveBackwardKey;
}
bool isMoveLeftKey(int key){
	return key == moveLeftKey;
}
bool isMoveRightKey(int key){
	return key == moveRightKey;
}
bool isCrouchKey(int key){
	return key == crouchKey; // left-ctrl
}
bool isInteractKey(int key){
	return key == interactKey;
}

bool isFireButton(int button){
	return button == fireButton;
}
bool isAimButton(int button){
	return button == aimButton;
}


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

	HotkeyToMessage {
		.key = '4',  // 3
		.action = 0,
		.keyToPublish = "interact-ingame-ui",
		.valueToPublish = "",
	},
	HotkeyToMessage {
		.key = '5',  // 3
		.action = 0,
		.keyToPublish = "ui-debug-text",
		.valueToPublish = "textvalue",
	},

};

void handleHotkey(int key, int action){
	for (auto &hotkey : hotkeys){
		if (hotkey.key == key && hotkey.action == action){
			gameapi -> sendNotifyMessage(hotkey.keyToPublish, hotkey.valueToPublish);
		}
	}
}
