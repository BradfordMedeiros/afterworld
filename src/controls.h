#ifndef MOD_AFTERWORLD_CONTROLS
#define MOD_AFTERWORLD_CONTROLS

#include "./util.h"
#include "./inventory.h"
#include "./weapons/weapon.h"
#include "./in-game-ui.h"


bool isJumpKey(int key);
bool isGrindKey(int key);
bool isReverseGrindKey(int key);
bool isMoveForwardKey(int key);
bool isMoveBackwardKey(int key);
bool isMoveLeftKey(int key);
bool isMoveRightKey(int key);
bool isCrouchKey(int key);
bool isClimbKey(int key);
bool isInteractKey(int key);
bool isPauseKey(int key);

bool isFireButton(int button);
bool isAimButton(int button);
bool isModifierButton(int button);
bool isTeleportButton(int button);
bool isExitTerminalKey(int button);
bool isToggleThirdPersonKey(int button);

void handleHotkey(int key, int action);

#endif