#ifndef MOD_AFTERWORLD_COMPONENTS_HUD
#define MOD_AFTERWORLD_COMPONENTS_HUD

#include "../common.h"
#include "../../../global.h"

struct AmmoHudInfo {
  int currentAmmo;
  int totalAmmo;
};
void setUIAmmoCount(AmmoHudInfo ammoInfo);
void setHealth(float health);
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

