#include "./controls.h"

extern CustomApiBindings* gameapi;
extern Weapons weapons;

std::optional<objid> getPlayerId();


int jumpKey = 32;
int grindKey = 'Q';
int reverseGrindKey = 'Z';
int moveFowardKey = 'W';
int moveBackwardKey = 'S';
int moveLeftKey = 'A';
int moveRightKey = 'D';
int crouchKey = 341;
int interactKey = 'E';

int fireButton = 0;
int aimButton = 1;
int modifierButton = 'I';  // shift
int teleportButton = 'T';
int exitTerminalButton = 'R';
int toggleThirdPersonButton = 'O';

void setControl(int controlSymbol, int key){
	static int jumpSymbol = getSymbol("control-jump");
	static int grindSymbol = getSymbol("control-grind");
	static int reverseGrindSymbol = getSymbol("control-grind-reverse");
	static int moveForwardSymbol = getSymbol("control-move-forward");
	static int moveBackwardSymbol = getSymbol("control-move-backward");
	static int moveLeftSymbol = getSymbol("control-move-left");
	static int moveRightSymbol = getSymbol("control-move-right");
	static int crouchSymbol = getSymbol("control-crouch");
	static int interactSymbol = getSymbol("control-interact");

	static int fireSymbol = getSymbol("control-fire");
	static int aimSymbol = getSymbol("control-aim");
	static int modifierSymbol = getSymbol("control-modifier");
	static int teleportSymbol = getSymbol("control-teleport");
	static int exitTerminalSymbol = getSymbol("control-exit-terminal");
	static int toggleThirdPersonSymbol = getSymbol("control-third-person");

	if (controlSymbol == jumpSymbol){
		jumpKey = key;
	}else if (controlSymbol == grindSymbol){
		grindKey = key;
	}else if (controlSymbol == reverseGrindSymbol){
		grindKey = key;
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
	}else if (controlSymbol == modifierSymbol){
		modifierButton = key;
	}else if (controlSymbol == teleportSymbol){
		teleportButton = key;
	}else if (controlSymbol == exitTerminalSymbol){
		exitTerminalSymbol = key;
	}else if (controlSymbol == toggleThirdPersonSymbol){
		toggleThirdPersonSymbol = key;
	}else {
		modassert(false, std::string("invalid control symbol: ") + nameForSymbol(controlSymbol));
	}
}

bool isJumpKey(int key){
	return key == jumpKey; // space
}
bool isGrindKey(int key){
	return key == grindKey;
}
bool isReverseGrindKey(int key){
	return key == reverseGrindKey;
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
bool isClimbKey(int key){
	return key == 'R';
}
bool isInteractKey(int key){
	return key == interactKey;
}
bool isPauseKey(int key){
	return key == 256; // escape 
}

bool isFireButton(int button){
	return button == fireButton;
}
bool isAimButton(int button){
	return button == aimButton;
}
bool isModifierButton(int button){
	return button == modifierButton;
}
bool isTeleportButton(int button){
	return button == teleportButton;
}
bool isExitTerminalKey(int button){
	return button == exitTerminalButton;
}
bool isToggleThirdPersonKey(int button){
	return button == toggleThirdPersonButton;
}

struct HotkeyToMessage {
	int key;
	std::optional<int> action;
	std::function<void()> fn;
};

void maybeChangeGunUpdateUi(const char* gun){
	if (getPlayerId().has_value()){
		maybeChangeGun(getWeaponState(weapons, getPlayerId().value()), gun,  getPlayerId().value(), getPlayerId().value());
	}
}

bool animationExists(objid entityId, const char* animationName){
  auto animationNames = gameapi -> listAnimations(entityId);
  for (auto &animation : animationNames){
    if (animation == animationName){
      return true;
    }
  }
  return false;
}

std::vector<HotkeyToMessage> hotkeys = {
	HotkeyToMessage {
		.key = 48,  // 0
		.action = 0,
		.fn = []() -> void {
			maybeChangeGunUpdateUi("none");
		},
	},
	HotkeyToMessage {
		.key = '1',  // 1
		.action = 0,
		.fn = []() -> void {
			maybeChangeGunUpdateUi("pistol");
		},
	},
	HotkeyToMessage {
		.key = '2',  // 2 
		.action = 0,
		.fn = []() -> void {
			maybeChangeGunUpdateUi("electrogun");
		},
	},
	HotkeyToMessage {
		.key = '3',  // 3
		.action = 0,
		.fn = []() -> void {
			maybeChangeGunUpdateUi("scrapgun");
		},
	},
	HotkeyToMessage {
		.key = '4',  
		.action = 0,
		.fn = []() -> void {
			maybeChangeGunUpdateUi("fork");
		},
	},

	HotkeyToMessage {
		.key = '9',  
		.action = 0,
		.fn = []() -> void {
			modlog("controls", "set pose");
			const char* pose = "pose-t";
			if (animationExists(getPlayerId().value(), pose)){
				modlog("animation", std::string("set pose: ") + std::string(pose));
				gameapi -> setAnimationPose(getPlayerId().value(), pose, 0.f);
			}else{
				modlog("animation", std::string("set pose does not exist: ") + std::string(pose));
				modassert(false, "animation does not exist");
			}
		},
	},


  
	

};

void handleHotkey(int key, int action){
	for (auto &hotkey : hotkeys){
		if (hotkey.key == key && hotkey.action == action){
			hotkey.fn();
		}
	}
}
