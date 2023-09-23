#ifndef MOD_AFTERWORLD_MENUS
#define MOD_AFTERWORLD_MENUS

#include <string>
#include <vector>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../components/nestedlist.h"
#include "../components/basic/layout.h"

#include "../../util.h"
#include "../../global.h"


struct NavListApi {
	std::function<void(std::string)> changeLayout;
};
extern Component navList;

#endif