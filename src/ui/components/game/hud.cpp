#include "./hud.h"

extern CustomApiBindings* gameapi;

struct AmmoHudInfo {
  int currentAmmo;
  int totalAmmo;
};

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

enum DrawBarAlign { DRAWBAR_ALIGN_NONE, DRAWBAR_ALIGN_POSITIVE, DRAWBAR_ALIGN_NEGATIVE };
void drawbar(DrawingTools& drawTools, float percentage, glm::vec2 sizeNdi, glm::vec2 offset, DrawBarAlign align, glm::vec4 tint){
  float width = sizeNdi.x;
  float widthPercentage = glm::min(1.f, glm::max(0.f, percentage));

  float barHeight = sizeNdi.y;

  float alignOffset = 0.f;
  if (align == DRAWBAR_ALIGN_NEGATIVE){
    alignOffset = barHeight * -0.5f;
  }else if (align == DRAWBAR_ALIGN_POSITIVE){
    alignOffset = barHeight * 0.5f;
  }

  drawTools.drawRect(offset.x, offset.y + alignOffset, width, barHeight, false, glm::vec4(0.2f, 0.2f, 0.2f, 0.5f),true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  drawTools.drawRect(offset.x, offset.y + alignOffset, widthPercentage * width, barHeight, false, tint, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  //modlog("ui health percentage", std::to_string(widthPercentage));
}

//std::optional<std::string> imageForHud = "./res/textures/badhud.png";
std::optional<std::string> imageForHud = std::nullopt;

Component hudComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    if (imageForHud.has_value()){
      drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), true, std::nullopt, imageForHud.value(), std::nullopt, std::nullopt);
    }

    drawTools.drawText(std::string("ammo: ") + std::to_string(ammoInfo.currentAmmo) + " / " + std::to_string(ammoInfo.totalAmmo), 0.85, 0.95, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);

    if (uiHealth.has_value()){
      drawbar(drawTools, uiHealth.value().health / uiHealth.value().totalHealth, glm::vec2(1.f, 0.1f), glm::vec2(0.f, 1.f), DRAWBAR_ALIGN_NEGATIVE, glm::vec4(0.f, 0.f, 1.f, 0.5f));
      drawTools.drawText("health: " + std::to_string(static_cast<int>(uiHealth.value().health)), 0.85f, 0.9f, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
    }

    drawTools.drawText(std::string("weapon: ") + (uiWeapon.has_value() ? uiWeapon.value() : std::string("unequipped")), 0.65, 0.75, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);

    if (uiGemCount.has_value()){
      drawTools.drawText(std::string("gem count: ") + (uiGemCount.has_value() ? (std::to_string(uiGemCount.value().currentCount) + " / " + std::to_string(uiGemCount.value().totalCount)) : ""), 0.65, 0.65, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
    }

    if (uiVelocity.has_value()){
      glm::ivec3 speedRounded(uiVelocity.value().x, uiVelocity.value().y, uiVelocity.value().z);
      drawTools.drawText(std::string("speed: ") + print(speedRounded), 0.65, 0.95, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
     
      auto speed = glm::length(uiVelocity.value());
      drawbar(drawTools, speed / 250.f, glm::vec2(2.f, 0.025f), glm::vec2(0.f, -1.f), DRAWBAR_ALIGN_POSITIVE, glm::vec4(1.f, 1.f, 1.f, 0.2f));
    }

    if (uiLookVelocity.has_value()){
      drawTools.drawLine2D(glm::vec3(0.f, 0.f, 0.f), glm::vec3(uiLookVelocity.value().x, uiLookVelocity.value().y, 0.f), false, glm::vec4(0.f, 0.f, 1.f, 0.8f), true, std::nullopt, std::nullopt, std::nullopt);
    }

    if (showActivate){
      drawTools.drawText("press e to activate", 0.75f, 0.7f, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
    }

  	return BoundingBox2D {
  		.x = 0.f,
  		.y = 0.f,
  		.width = 2.f,
  		.height = 2.f,
  	};
  },
};

