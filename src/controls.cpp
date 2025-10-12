#include "./controls.h"

extern CustomApiBindings* gameapi;
extern Weapons weapons;

std::optional<objid> getPlayerId(int playerIndex);
int getDefaultPlayerIndex();

void maybeRemoveControllableEntity(objid idRemoved);
void createHitbox(objid id);
void enterRagdoll(objid id);

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
int reloadButton = 'H';

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
	static int reloadSymbol = getSymbol("control-reload");

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
		exitTerminalButton = key;
	}else if (controlSymbol == toggleThirdPersonSymbol){
		toggleThirdPersonButton = key;
	}else if (controlSymbol == reloadSymbol){
		reloadButton = key;
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

bool isReloadKey(int button){
	return button == reloadButton;
}

struct HotkeyToMessage {
	int key;
	std::optional<int> action;
	std::function<void()> fn;
};

void maybeChangeGunUpdateUi(const char* gun){
	if (getPlayerId(getDefaultPlayerIndex()).has_value()){
		maybeChangeGun(getWeaponState(weapons, getPlayerId(getDefaultPlayerIndex()).value()), gun,  getPlayerId(getDefaultPlayerIndex()).value());
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


void testPhysicsObjects(){
	static bool firstTime = true;

	auto box1 = findObjByShortName("boxtest1", std::nullopt);
	auto box2 = findObjByShortName("boxtest2", std::nullopt);

	if (firstTime){

		// create a custom rigidbody, not for a bone necessarily
		rigidBodyOpts physicsOptions1 {
		  .linear = glm::vec3(1.f, 1.f, 1.f),
		  .angular = glm::vec3(0.f, 1.f, 0.f),
		  .gravity = glm::vec3(0.f, -10.f, 0.f),
		  .friction = 0.f,
		  .restitution = 1.f,
		  .mass = 1.f,
		  .layer = 0,
		  .linearDamping = 0.f,
		  .isStatic = false,
		  .hasCollisions = true,
		};
		rigidBodyOpts physicsOptions2 {
		  .linear = glm::vec3(0.f, 0.f, 0.f),
		  .angular = glm::vec3(0.f, 0.f, 0.f),
		  .gravity = glm::vec3(0.f, -10.f, 0.f),
		  .friction = 0.f,
		  .restitution = 1.f,
		  .mass = 100.f,
		  .layer = 0,
		  .linearDamping = 0.f,
		  .isStatic = false,
		  .hasCollisions = true,
		};

  	//BoundInfo boundInfo;
  	//Transformation transformation;
  	//std::optional<glm::vec3> offset;

		PhysicsCreateSphere sphereShape {
			.radius = 5.f,
		};


		auto box1Bounding = gameapi -> getPhysicsInfo(box1.value(), false);
		auto boundInfo1 = box1Bounding.value().boundInfo;
		PhysicsCreateRect rectShape1 {
			.width = boundInfo1.xMax - boundInfo1.xMin,
			.height = boundInfo1.yMax - boundInfo1.yMin,
			.depth = boundInfo1.zMax - boundInfo1.zMin,
		};

		auto box2Bounding = gameapi -> getPhysicsInfo(box2.value(), false);
		auto boundInfo2 = box2Bounding.value().boundInfo;
		PhysicsCreateRect rectShape2 {
			.width = boundInfo2.xMax - boundInfo2.xMin,
			.height = boundInfo2.yMax - boundInfo2.yMin,
			.depth = boundInfo2.zMax - boundInfo2.zMin,
		};
		{
			gameapi -> createPhysicsBody(box1.value(), rectShape1);
			gameapi -> setPhysicsOptions(box1.value(), physicsOptions1);
		}
		{
			gameapi -> createPhysicsBody(box2.value(), rectShape2);
			gameapi -> setPhysicsOptions(box2.value(), physicsOptions2);
		}
	}else{
		//gameapi -> createFixedConstraint(box1.value(), box2.value());
		gameapi -> createPointConstraint(box1.value(), box2.value());
		//gameapi -> createHingeConstraint(box1.value(), box2.value());
	}
	firstTime = false;
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
		.key = '5',  
		.action = 0,
		.fn = []() -> void {
			maybeChangeGunUpdateUi("launcher");
		},
	},
	HotkeyToMessage {
		.key = '6',  
		.action = 0,
		.fn = []() -> void {
			maybeChangeGunUpdateUi("link");
		},
	},
	HotkeyToMessage {
		.key = '7',  
		.action = 0,
		.fn = []() -> void {
			maybeChangeGunUpdateUi("shotgun");
		},
	},

	HotkeyToMessage {
		.key = '8',  
		.action = 0,
		.fn = []() -> void {
			//testPhysicsObjects();

			/*static bool bonesCreated = false;
			if (!bonesCreated){
				printLayerInfo();
				createHitbox(getPlayerId().value());
				bonesCreated = true;
			}else{
				enterRagdoll(getPlayerId().value());
			}*/

			// set pose
			/*static bool setPose = false;
			setPose = !setPose;
			modlog("controls", "set pose");
			if(setPose){
				const char* pose = "sitting";
				if (animationExists(getPlayerId().value(), pose)){
					modlog("animation", std::string("set pose: ") + std::string(pose));
					gameapi -> setAnimationPose(getPlayerId().value(), pose, 0.f);
				}else{
					modlog("animation", std::string("set pose does not exist: ") + std::string(pose));
					modassert(false, "animation does not exist");
				}
			}else{
				gameapi -> clearAnimationPose(getPlayerId().value());
			}*/
		},
	},
	// HotkeyToMessage {
	// 	.key = '9',  
	// 	.action = 0,
	// 	.fn = []() -> void {
	// 		auto playerModel = getPlayerId().value();
	// 		auto headValue = findChildObjBySuffix(playerModel, "Head");
	// 		rigidBodyOpts physicsOptions {
	// 		  .linear = glm::vec3(1.f, 1.f, 1.f),
	// 		  .angular = glm::vec3(0.f, 0.f, 0.f),
	// 		  .gravity = glm::vec3(0.f, -9.81f, 0.f),
	// 		  .friction = 0.f,
	// 		  .restitution = 1.f,
	// 		  .mass = 10.f,
	// 		  .layer = 2,
	// 		  .velocity= std::nullopt,
	// 		  .angularVelocity = std::nullopt,
	// 		  .linearDamping = 0.f,
	// 		  .isStatic = false,
	// 		  .hasCollision = true,
	// 		};
	// 		gameapi -> setPhysicsOptions(headValue.value(), physicsOptions);
	// 		/*static bool setPose = false;
	// 		setPose = !setPose;
	// 		modlog("controls", "set pose");
	// 		if(setPose){
	// 			const char* pose = "sitting";
	// 			if (animationExists(getPlayerId().value(), pose)){
	// 				modlog("animation", std::string("set pose: ") + std::string(pose));
	// 				gameapi -> setAnimationPose(getPlayerId().value(), pose, 0.f);
	// 			}else{
	// 				modlog("animation", std::string("set pose does not exist: ") + std::string(pose));
	// 				modassert(false, "animation does not exist");
	// 			}
	// 		}else{
	// 			gameapi -> clearAnimationPose(getPlayerId().value());
	// 		}*/
	// 	},
	// },

};

void handleHotkey(int key, int action){
	for (auto &hotkey : hotkeys){
		if (hotkey.key == key && hotkey.action == action){
			hotkey.fn();
		}
	}
}


///////////////////////

RemappedKey remapDeviceKeys(int key, int scancode, int action, int mods){
	bool forceSecondPlayer = false;
	if (key == 265){  // up arrow
		forceSecondPlayer = true;
		key = 'W';
	}
	if (key == 264){ 
		forceSecondPlayer = true;
		key = 'S';
	}
	if (key == 263){  
		forceSecondPlayer = true;
		key = 'A';
	}
	if (key == 262){ 
		forceSecondPlayer = true;
		key = 'D';
	}
  return RemappedKey {
    .playerPort = forceSecondPlayer ? 1 : getDefaultPlayerIndex(),
    .key = key,
    .scancode = scancode,
    .action = action,
    .mods = mods,
  };
}

RemappedMouseMovement remapMouseMovement(double xPos, double yPos, float xNdc, float yNdc){
  return RemappedMouseMovement {
    .playerPort = getDefaultPlayerIndex(),
    .xPos = xPos,
    .yPos = yPos,
    .xNdc = xNdc,
    .yNdc = yNdc,
  };
}

RemappedMouseCallback remapMouseCallback(int button, int action, int mods){
  return RemappedMouseCallback {
    .playerPort = getDefaultPlayerIndex(),
    .button = button,
    .action = action,
    .mods = mods,
  };
}

RemappedScrollCallback remapScrollCallback(double amount){
  return RemappedScrollCallback {
    .playerPort = getDefaultPlayerIndex(),
    .amount = amount,
  };
}
