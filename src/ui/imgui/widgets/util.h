#pragma once

#include "../../../../../ModEngine/src/ui/gui.h"
#include "../../../../src/ui/components/console_core.h"
#include "../../../../src/ai/ai.h"
#include "../../../../src/gamecontrol/gametypes.h"
#include "../../../../src/gamecontrol/entity.h"

void renderConsole(bool includePanel);

// This is a bit different not a widget, but this just sponors the drawing of the text
void onAlertFrame();
void pushAlertMessage(std::string message);


void renderAnimations(bool includePanel);
void renderGameType(bool includePanel);
void renderHitpoints(bool includePanel);
void renderInventory(bool includePanel);