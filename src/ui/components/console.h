#ifndef MOD_AFTERWORLD_COMPONENTS_CONSOLE
#define MOD_AFTERWORLD_COMPONENTS_CONSOLE

#include <deque>

#include "./common.h"
#include "./basic/layout.h"
#include "./basic/listitem.h"
#include "./basic/textbox.h"

struct ConsoleInterface {
  std::function<void(bool)> setShowEditor;
};

extern Component consoleComponent;

#endif

