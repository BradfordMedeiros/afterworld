#pragma once

#include "../../../../../ModEngine/src/ui/gui.h"
#include "../../../scene_routing.h"
#include "../common.h"

void renderGameSettingsControlPanel(bool includePanel);
void renderGameSettingsView(bool includePanel);

void renderMainMenu(bool includePanel);

struct LiveMenuFn {
  LiveMenu* liveMenu = NULL;
};
void renderMainMenu2(bool includePanel, LiveMenuFn& liveMenu);

void renderPauseMenu(bool includePanel);
void renderDeadMenu(bool includePanel);

void renderLevelList(bool includePanel);
void renderLevelDetail(bool includePanel);