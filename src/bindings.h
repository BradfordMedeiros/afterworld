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
#include "./global.h"
#include "./util/components/common.h"
#include "./util/components/list.h"
#include "./util/components/radiobutton.h"
#include "./util/components/slider.h"
#include "./util/components/imagelist.h"
#include "./util/components/router.h"
#include "./util/views/mainui.h"
#include "./util/views/pausemenu.h"

std::vector<CScriptBinding> getUserBindings(CustomApiBindings& api);

#endif