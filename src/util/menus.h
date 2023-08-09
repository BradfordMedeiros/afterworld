#ifndef MOD_AFTERWORLD_MENUS
#define MOD_AFTERWORLD_MENUS

#include <string>
#include <vector>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "./components/nestedlist.h"
#include "./components/imagelist.h"
#include "./components/slider.h"
#include "./components/radiobutton.h"
#include "./components/layout.h"

#include "../util.h"
#include "../global.h"


std::vector<ImListItem> createPauseMenu(std::function<void()> resume, std::function<void()> goToMainMenu);


extern std::vector<NestedListItem> nestedListTest;
extern Component radioButtonSelector;
extern Component nestedListTestComponent;
extern Component testLayoutComponent;

#endif