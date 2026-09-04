#include "./hud.h"

extern CustomApiBindings* gameapi;

AmmoHudInfo ammoInfo {
  .currentAmmo = 0,
  .totalAmmo = 0,
};
void setUIAmmoCount(int currentAmmo, int totalAmmo){
  ammoInfo =  AmmoHudInfo {
    .currentAmmo = currentAmmo,
    .totalAmmo = totalAmmo,
  };
}

std::optional<UiHealth> uiHealth;
void setUiHealth(int player, std::optional<UiHealth> health){
  uiHealth = health;
  //modlog("ui health", std::to_string(health) + ", " + std::to_string(totalHealth));
}

std::optional<glm::vec3> uiVelocity;
std::optional<glm::vec2> uiLookVelocity;
void setUiSpeed(std::optional<glm::vec3> velocity, std::optional<glm::vec2> lookVelocity){
  uiVelocity = velocity;
  uiLookVelocity = lookVelocity;
}

bool showActivate = false;
void setShowActivate(bool show){
  showActivate = show;
}

std::optional<std::string> uiWeapon;
void setUiWeapon(std::optional<std::string> weapon){
  uiWeapon = weapon;
}

std::optional<GemCount> uiGemCount;
void setUiGemCount(std::optional<GemCount> count){
  uiGemCount = count;
}

std::optional<float> zoomAmount;
void setZoomAmount(std::optional<float> amount){
  zoomAmount = amount;
}

//std::optional<std::string> imageForHud = "./res/textures/badhud.png";
std::optional<std::string> imageForHud = std::nullopt;


