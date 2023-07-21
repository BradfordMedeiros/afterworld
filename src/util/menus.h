#ifndef MOD_AFTERWORLD_MENUS
#define MOD_AFTERWORLD_MENUS

#include <string>
#include <vector>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "./drawmenu.h"
#include "../util.h"
#include "../global.h"

extern std::vector<NestedListItem> nestedListTest;

std::vector<ImListItem> createPauseMenu(std::function<void()> resume, std::function<void()> goToMainMenu);
std::vector<RadioButton> createRadioButtons();

#endif