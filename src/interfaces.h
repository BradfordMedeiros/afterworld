#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"
#include "./arcade/arcade.h"
#include "./ui/views/uicontext.h"
#include "./ai/ai.h"
#include "./core/movement/movement.h"
#include "./core/health.h"
#include "./interaction/terminal.h"
#include "./debug.h"
#include "./gamecontrol/gametypes/gametypes.h"

ArcadeApi createArcadeApi();
extern AIInterface aiInterface;
extern Weapons weapons;

UiContext getUiContext();