#pragma once

#include "../../../../../ModEngine/src/ui/gui.h"
#include "../../../scene_routing.h"

void renderTraitsPanel(bool includePanel);
void renderWeaponsPanel(bool includePanel);
void renderSpawnPanel(bool includePanel);
void renderPropPanel(bool includePanel, std::optional<objid> sceneId);



void renderFpsHud(bool includePanel);