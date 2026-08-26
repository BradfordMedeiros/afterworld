#include "./controls.h"

extern CustomApiBindings* gameapi;
extern Weapons weapons;

std::optional<objid> getPlayerId(int playerIndex);
int getDefaultPlayerIndex();
bool entityInVehicle(objid);

void maybeRemoveControllableEntity(objid idRemoved);
void createHitbox(objid id);
void enterRagdoll(objid id);

int jumpKey = GLFW_KEY_SPACE;
int grindKey = GLFW_KEY_Q;
int reverseGrindKey = GLFW_KEY_Z;

int moveFowardKey = GLFW_KEY_W;
int moveBackwardKey = GLFW_KEY_S;
int moveLeftKey = GLFW_KEY_A;
int moveRightKey = GLFW_KEY_D;

int crouchKey = GLFW_KEY_LEFT_CONTROL;
int interactKey = GLFW_KEY_E;
int pausekey = GLFW_KEY_ESCAPE;

int fireButton = GLFW_MOUSE_BUTTON_LEFT;
int aimButton = GLFW_MOUSE_BUTTON_RIGHT;

int modifierButton = GLFW_KEY_LEFT_SHIFT;
int teleportButton = GLFW_KEY_T;
int exitTerminalButton = GLFW_KEY_R;
int toggleThirdPersonButton = GLFW_KEY_O;
int reloadButton = GLFW_KEY_H;


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
	return key == pausekey;
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
	auto playerId = getPlayerId(getDefaultPlayerIndex());
	if (playerId.has_value() && !entityInVehicle(playerId.value())){
		maybeChangeGun(getWeaponState(weapons, playerId.value()), gun,  getPlayerId(getDefaultPlayerIndex()).value());
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
			gameapi -> createPhysicsBody(box1.value(), rectShape1, std::nullopt);
			gameapi -> setPhysicsOptions(box1.value(), physicsOptions1);
		}
		{
			gameapi -> createPhysicsBody(box2.value(), rectShape2, std::nullopt);
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
	//if (key == 265){  // up arrow
	//	forceSecondPlayer = true;
	//	key = 'W';
	//}
	//if (key == 264){ 
	//	forceSecondPlayer = true;
	//	key = 'S';
	//}
	//if (key == 263){  
	//	forceSecondPlayer = true;
	//	key = 'A';
	//}
	//if (key == 262){ 
	//	forceSecondPlayer = true;
	//	key = 'D';
	//}
  return RemappedKey {
    .playerPort = forceSecondPlayer ? 1 : getDefaultPlayerIndex(),
    .key = key,
    .scancode = scancode,
    .action = action,
    .mods = mods,
  };
}


void maybeAddThreshold(std::vector<RemappedKey>& _keys, float lastFrameValue, float thisFrameValue, float threshold, int joystick, int key){
	if (threshold < 0.f){
		if (lastFrameValue > threshold && thisFrameValue <= threshold){
			_keys.push_back(
				RemappedKey {
					.playerPort = joystick,
					.key = key,
					.scancode = key,
					.action = 1,
					.mods = 0,
				}
  	  );
  	  return;
		}
		if (lastFrameValue <= threshold && thisFrameValue > threshold){
			_keys.push_back(
				RemappedKey {
					.playerPort = joystick,
					.key = key,
					.scancode = key,
					.action = 0,
					.mods = 0,
				}
  	  );
  	  return;
		}
	}


	if (lastFrameValue < threshold && thisFrameValue >= threshold){
		_keys.push_back(
			RemappedKey {
				.playerPort = joystick,
				.key = key,
				.scancode = key,
				.action = 1,
				.mods = 0,
			}
    );
    return;
	}
	if (lastFrameValue >= threshold && thisFrameValue < threshold){
		_keys.push_back(
				RemappedKey {
					.playerPort = joystick,
					.key = key,
					.scancode = key,
					.action = 0,
					.mods = 0,
				}
    );
    return;
	}
}
std::vector<RemappedKey> remapFrameToKeys(int joystick, ControlInfo2& controls){
	std::vector<RemappedKey> keys;

	// This is kind of lame since not actual analog but dpad like, but it's OK until further down the line
	maybeAddThreshold(keys, controls.lastFrame.axisInfo.leftStickY, controls.thisFrame.axisInfo.leftStickY, -0.6, joystick, moveFowardKey);
	maybeAddThreshold(keys, controls.lastFrame.axisInfo.leftStickY, controls.thisFrame.axisInfo.leftStickY, 0.6, joystick, moveBackwardKey);
	maybeAddThreshold(keys, controls.lastFrame.axisInfo.leftStickX, controls.thisFrame.axisInfo.leftStickX, -0.6, joystick, moveLeftKey);
	maybeAddThreshold(keys, controls.lastFrame.axisInfo.leftStickX, controls.thisFrame.axisInfo.leftStickX, 0.6, joystick, moveRightKey);


	return keys;
}

void maybeAddThreshold(std::vector<RemappedMouseCallback>& _keys, float lastFrameValue, float thisFrameValue, float threshold, int joystick, int button){
	if (threshold < 0.f){
		if (lastFrameValue > threshold && thisFrameValue <= threshold){
			_keys.push_back(
				RemappedMouseCallback {
  	  		.playerPort = joystick,
  	  		.button = button,
  	  		.action = 1,
  	  		.mods = 0,
  	  	}
  	  );
  	  return;
		}
		if (lastFrameValue <= threshold && thisFrameValue > threshold){
			_keys.push_back(
				RemappedMouseCallback {
  	  		.playerPort = joystick,
  	  		.button = button,
  	  		.action = 0,
  	  		.mods = 0,
  	  	}
  	  );
  	  return;
		}
	}


	if (lastFrameValue < threshold && thisFrameValue >= threshold){
		_keys.push_back(
			RemappedMouseCallback {
    		.playerPort = joystick,
    		.button = button,
    		.action = 1,
    		.mods = 0,
    	}
    );
    return;
	}
	if (lastFrameValue >= threshold && thisFrameValue < threshold){
		_keys.push_back(
			RemappedMouseCallback {
    		.playerPort = joystick,
    		.button = button,
    		.action = 0,
    		.mods = 0,
    	}
    );
    return;
	}
}

std::vector<RemappedMouseCallback> remapFrameToMouse(int joystick, ControlInfo2& controls){
	std::vector<RemappedMouseCallback> keys;
	maybeAddThreshold(keys, controls.lastFrame.axisInfo.rightTrigger, controls.thisFrame.axisInfo.rightTrigger, 0.6f, joystick, fireButton);
	maybeAddThreshold(keys, controls.lastFrame.axisInfo.leftTrigger, controls.thisFrame.axisInfo.leftTrigger, 0.6, joystick, aimButton);

	return keys;
}

std::optional<RemappedKey> remapControllerToKeys(int joystick, BUTTON_TYPE button, bool keyDown){
	if (button == BUTTON_A && keyDown){
		return RemappedKey {
			.playerPort = joystick,
			.key = jumpKey,
			.scancode = jumpKey,
			.action = 1,
			.mods = 0,
		};
	}
	if (button == BUTTON_B && keyDown){
		return RemappedKey {
			.playerPort = joystick,
			.key = toggleThirdPersonButton,
			.scancode = toggleThirdPersonButton,
			.action = 1,
			.mods = 0,
		};
	}
	if (button == BUTTON_Y && keyDown){
		return RemappedKey {
			.playerPort = joystick,
			.key = modifierButton,
			.scancode = modifierButton,
			.action = 1,
			.mods = 0,
		};
	}

	if (button == BUTTON_LB && keyDown){
		return RemappedKey {
			.playerPort = joystick,
			.key = 'Q',
			.scancode = 'Q',
			.action = 0,
			.mods = 0,
		};		
	}	

	if (button == BUTTON_X && keyDown){
		return RemappedKey {
			.playerPort = joystick,
			.key = interactKey,
			.scancode = interactKey,
			.action = 1,
			.mods = 0,
		};		
	}
	if (button == BUTTON_START && keyDown){
		return RemappedKey {
			.playerPort = joystick,
			.key = pausekey,
			.scancode = pausekey,
			.action = 1,
			.mods = 0,
		};		
	}
	if (button == BUTTON_UP){
		return RemappedKey {
			.playerPort = joystick,
			.key = moveFowardKey,
			.scancode = moveFowardKey,
			.action = (keyDown ? 1 :  0),
			.mods = 0,
		};
	}
	if (button == BUTTON_DOWN){
		return RemappedKey {
			.playerPort = joystick,
			.key = moveBackwardKey,
			.scancode = moveBackwardKey,
			.action = (keyDown ? 1 :  0),
			.mods = 0,
		};
	}
	if (button == BUTTON_LEFT){
		return RemappedKey {
			.playerPort = joystick,
			.key = moveLeftKey,
			.scancode = moveLeftKey,
			.action = (keyDown ? 1 :  0),
			.mods = 0,
		};
	}
	if (button == BUTTON_RIGHT){
		return RemappedKey {
			.playerPort = joystick,
			.key = moveRightKey,
			.scancode = moveRightKey,
			.action = (keyDown ? 1 :  0),
			.mods = 0,
		};
	}


	return std::nullopt;
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

bool keyIsDown(int key){
  extern GLFWwindow* window;
  return glfwGetKey(window, key) == GLFW_PRESS;
}

bool leftMouseDown(){
  return getGlobalState().control.leftMouseDown;
}
bool rightMouseDown(){
  return getGlobalState().control.rightMouseDown;
}
bool middleMouseDown(){
  return getGlobalState().control.middleMouseDown;
}
glm::vec2 getMouseVelocity(){
  return getGlobalState().control.mouseVelocity;
}

std::vector<ControlBinding> controlBindings(){
	return {
		ControlBinding { .text = "Jump", .currentKey = &jumpKey },  
		ControlBinding { .text = "Forward", .currentKey = &moveFowardKey },  
		ControlBinding { .text = "Back", .currentKey = &moveBackwardKey },  
		ControlBinding { .text = "Left", .currentKey = &moveLeftKey },  
		ControlBinding { .text = "Right", .currentKey = &moveRightKey },  

	};
}