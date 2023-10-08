#ifndef MOD_AFTERWORLD_BINDING
#define MOD_AFTERWORLD_BINDING

#include <iostream>
#include <vector>
#include <stack>
#include "./ai/ai.h"
#include "./movement.h"
#include "./menu.h"
#include "./daynight.h"
#include "./weapon.h"
#include "./vehicle.h"
#include "./inventory.h"
#include "./hud.h"
#include "./tags.h"
#include "./dialog.h"
#include "./debug.h"
#include "./weather.h"
#include "./in-game-ui.h"
#include "./sound.h"
#include "./water.h"
#include "./gametypes.h"
#include "./spawn.h"
#include "./global.h"
#include "./ui/components/common.h"
#include "./ui/components/basic/list.h"
#include "./ui/components/basic/radiobutton.h"
#include "./ui/components/basic/slider.h"
#include "./ui/components/imagelist.h"
#include "./ui/components/router.h"
#include "./ui/views/mainui.h"
#include "./ui/views/pausemenu.h"

std::vector<CScriptBinding> getUserBindings(CustomApiBindings& api);

#endif