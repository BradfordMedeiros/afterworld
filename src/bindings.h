#ifndef MOD_AFTERWORLD_BINDING
#define MOD_AFTERWORLD_BINDING

#include <iostream>
#include <vector>
#include <stack>
#include "./ai/ai.h"
#include "./entity.h"
#include "./gameworld/daynight.h"
#include "./weapons/weapon.h"
#include "./movement/movement.h"
#include "./inventory.h"
#include "./tags.h"
#include "./dialog.h"
#include "./debug.h"
#include "./weather.h"
#include "./in-game-ui.h"
#include "./resources/sound.h"
#include "./water.h"
#include "./gametypes/gametypes.h"
#include "./director/spawn.h"
#include "./director/director.h"
#include "./global.h"
#include "./ui/components/common.h"
#include "./ui/components/basic/list.h"
#include "./ui/components/basic/radiobutton.h"
#include "./ui/components/basic/slider.h"
#include "./ui/components/imagelist.h"
#include "./ui/components/router.h"
#include "./ui/views/mainui.h"
#include "./ui/views/pausemenu.h"
#include "./modelviewer.h"
#include "./collision.h"
#include "./cutscene.h"
#include "./curves.h"
#include "./vector_gfx.h"
#include "./gameworld/progress.h"
#include "./options.h"
#include "./vehicles.h"

#include "./arcade/arcade.h"

std::vector<CScriptBinding> getUserBindings(CustomApiBindings& api);
std::vector<const char*> getAdditionalPathsToValidate();


#endif