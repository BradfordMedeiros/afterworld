#ifndef MOD_AFTERWORLD_COMPONENTS_HUD
#define MOD_AFTERWORLD_COMPONENTS_HUD

#include "../common.h"
#include "../../../global.h"

void setUIAmmoCount(int currentAmmo, int totalAmmo);

struct UiHealth {
  float health;
  float totalHealth;
};

void setUiHealth(std::optional<UiHealth> health);
void setUiSpeed(std::optional<glm::vec3> speed);
void setShowActivate(bool showActivate);

struct LetterboxFade {
  std::string title;
  std::optional<float> animationDuration;
  std::optional<float> animationHold;
  std::optional<float> fadeOutDuration;
  glm::vec4 boxColor;
  glm::vec4 fadeColor;
  float fontSize;
};
void showLetterBox(std::string title, float duration);

extern Component hudComponent;

#endif

