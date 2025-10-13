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
bool isReloadKey(int button);

void handleHotkey(int key, int action);

//////////////////////
struct RemappedKey {
  int playerPort;
  int key;
  int scancode;
  int action;
  int mods;
};
RemappedKey remapDeviceKeys(int key, int scancode, int action, int mods);
struct RemappedMouseMovement {
  int playerPort;
  double xPos; 
  double yPos;
  float xNdc;
  float yNdc;
};
std::optional<RemappedKey> remapControllerToKeys(int joystick, BUTTON_TYPE button, bool keyDown);


RemappedMouseMovement remapMouseMovement(double xPos, double yPos, float xNdc, float yNdc);

struct RemappedMouseCallback {
  int playerPort;
  int button;
  int action;
  int mods;
};
RemappedMouseCallback remapMouseCallback(int button, int action, int mods);

struct RemappedScrollCallback {
  int playerPort;
  double amount;
};
RemappedScrollCallback remapScrollCallback(double amount);


#endif