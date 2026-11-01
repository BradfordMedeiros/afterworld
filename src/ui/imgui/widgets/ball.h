#pragma once

#include "../../../../../ModEngine/src/ui/gui.h"
#include "../../../scene_routing.h"
#include "../../../interaction/rhythm.h"

void renderBallGameplay(bool includePanel);
void renderBallProgressInfo(bool includePanel, BallModeUi& ballModeUi);

void renderStageSelectPanel(bool includePanel);
void renderRhythmPanel(bool includePanel);