#pragma once

#include "../../../../../ModEngine/src/ui/gui.h"
#include "../../../../src/ui/components/console_core.h"

void renderConsole(bool includePanel);

// This is a bit different not a widget, but this just sponors the drawing of the text
void onAlertFrame();
void pushAlertMessage(std::string message);
