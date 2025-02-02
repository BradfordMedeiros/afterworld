#ifndef MOD_AFTERWORLD_COMPONENTS_CONSOLE
#define MOD_AFTERWORLD_COMPONENTS_CONSOLE

#include <deque>

#include "./common.h"
#include "./basic/layout.h"
#include "./basic/listitem.h"
#include "./basic/textbox.h"

enum DebugPrintType {
  DEBUG_NONE,
  DEBUG_GLOBAL,
  DEBUG_INVENTORY,
  DEBUG_GAMETYPE,
  DEBUG_AI,
  DEBUG_HEALTH,
  DEBUG_ACTIVEPLAYER,
  DEBUG_ANIMATION,
};

struct ConsoleInterface {
  std::function<void()> setNormalMode;
  std::function<void()> setShowEditor;
  std::function<void()> setFreeCam;
  std::function<void(bool)> setNoClip;
  std::function<void(std::string)> setBackground;
  std::function<void(std::optional<std::string>)> goToLevel;
  std::function<void()> nextLevel;
  std::function<void(std::string)> takeScreenshot;

  std::function<void(std::string, bool)> routerPush;
  std::function<void()> routerPop;

  std::function<void()> die;
  std::function<void()> toggleKeyboard;
  std::function<void(DebugPrintType printType)> setShowDebugUi;
  std::function<void(bool)> showWeapon;
  std::function<void(int)> deliverAmmo;
  std::function<void(bool)> disableActiveEntity;
  std::function<void(std::string)> spawnByTag;
};

extern Component consoleComponent;

#endif

