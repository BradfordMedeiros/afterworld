#pragma once

#include "../../../../../ModEngine/src/ui/gui.h"
#include "../../../scene_routing.h"

void renderTraitsPanel(bool includePanel);
void renderWeaponsPanel(bool includePanel);
void renderSpawnPanel(bool includePanel);
void renderPropPanel(bool includePanel, std::optional<objid> sceneId);



void renderFpsHud(bool includePanel);

#include "../common.h"
#include "../../../global.h"
#include "../../../resources/paths.h"

struct AmmoHudInfo {
  int currentAmmo;
  int totalAmmo;
};
void setUIAmmoCount(int currentAmmo, int totalAmmo);

struct UiHealth {
  float health;
  float totalHealth;
};

void setUiHealth(int player, std::optional<UiHealth> health);
void setUiSpeed(std::optional<glm::vec3> velocity, std::optional<glm::vec2> lookVelocity);
void setShowActivate(bool showActivate);
void setUiWeapon(std::optional<std::string> weapon);

struct GemCount {
  int currentCount;
  int totalCount;
};
void setUiGemCount(std::optional<GemCount> count);
void setZoomAmount(std::optional<float> amount);

