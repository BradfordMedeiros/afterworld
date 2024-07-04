#ifndef MOD_AFTERWORLD_COMPONENTS_CONSOLE
#define MOD_AFTERWORLD_COMPONENTS_CONSOLE

#include <deque>

#include "./common.h"
#include "./basic/layout.h"
#include "./basic/listitem.h"
#include "./basic/textbox.h"

struct ConsoleInterface {
  std::function<void(bool)> setShowEditor;
  std::function<void(std::string)> setBackground;
  std::function<void(std::optional<std::string>)> goToLevel;

  std::function<void(std::string, bool)> routerPush;
  std::function<void()> routerPop;

  std::function<void()> die;
  std::function<void()> toggleKeyboard;
};

extern Component consoleComponent;

#endif

