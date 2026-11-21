#pragma once

#include <string>
#include <vector>
#include <optional>

#include "../../../../../ModEngine/src/ui/gui.h"
#include "../../../scene_routing.h"
#include "../../../interaction/rhythm.h"
#include "../../mainui.h"

void renderBallGameplay(bool includePanel);
void renderBallProgressInfo(bool includePanel, BallModeUi& ballModeUi);

void renderStageSelectPanel(bool includePanel);
void renderRhythmPanel(bool includePanel);

void renderSimpleTextDump(bool includePanel, const SimpleTextDisplayUi& display);