#ifndef MOD_AFTERWORLD_CONTROLS
#define MOD_AFTERWORLD_CONTROLS

#include "./util.h"
#include "./inventory.h"


bool isJumpKey(int key);
bool isMoveForwardKey(int key);
bool isMoveBackwardKey(int key);
bool isMoveLeftKey(int key);
bool isMoveRightKey(int key);
bool isCrouchKey(int key);
bool isInteractKey(int key);
bool isPauseKey(int key);

bool isFireButton(int button);
bool isAimButton(int button);

void handleHotkey(int key, int action);

#endif