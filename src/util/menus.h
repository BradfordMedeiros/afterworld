#ifndef MOD_AFTERWORLD_MENUS
#define MOD_AFTERWORLD_MENUS

#include <string>
#include <vector>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "./components/nestedlist.h"
#include "../util.h"
#include "../global.h"

extern std::vector<NestedListItem> nestedListTest;

std::vector<ImListItem> createPauseMenu(std::function<void()> resume, std::function<void()> goToMainMenu);

#endif