#ifndef MOD_AFTERWORLD_COMPONENTS_HUD
#define MOD_AFTERWORLD_COMPONENTS_HUD

#include "../common.h"
#include "../../../global.h"

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

extern Component hudComponent;


#endif

