#ifndef MOD_AFTERWORLD_BINDING
#define MOD_AFTERWORLD_BINDING

#include <iostream>
#include <vector>
#include <stack>
#include "./ai/scene_ai.h"
#include "./animation/scene_animation.h"
#include "./gamecontrol/entity.h"
#include "./gamecontrol/mode.h"
#include "./gameworld/daynight.h"
#include "./gameworld/surface.h"
#include "./gameworld/triggerzone.h"
#include "./core/weapons/weapon.h"
#include "./core/movement/movement.h"
#include "./core/inventory.h"
#include "./core/vehicles/vehicles.h"
#include "./core/scene_core.h"
#include "./tags.h"
#include "./interaction/dialog.h"
#include "./interaction/terminal.h"
#include "./interaction/cutscene.h"
#include "./interaction/in-game-ui.h"
#include "./debug.h"
#include "./gameworld/weather.h"
#include "./resources/sound.h"
#include "./gameworld/water.h"
#include "./gameworld/audio.h"
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
#include "./curves.h"
#include "./vector_gfx.h"
#include "./gameworld/progress.h"
#include "./options.h"
#include "./compile.h"
#include "./arcade/arcade.h"
#include "./scene_routing.h"
#include "./interfaces.h"


std::vector<CScriptBinding> getUserBindings(CustomApiBindings& api);
std::vector<const char*> getAdditionalPathsToValidate();
void setupGameCompileFn(std::unordered_map<std::string, std::string>& args);

#endif