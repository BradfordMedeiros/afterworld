#include "./hud.h"

AmmoHudInfo ammoInfo {
  .currentAmmo = 0,
  .totalAmmo = 0,
};
void setUIAmmoCount(AmmoHudInfo newAmmoInfo){
  ammoInfo = newAmmoInfo;
}

float currentHealth = 0.f;
void setHealth(float health){
  currentHealth = health;
}

void drawbar(DrawingTools& drawTools, float health){
  float width = 0.4f;
  float aspectRatio = 0.2f;
  float widthPercentage = glm::max(0.f, health / 100.f);
  float yNdc = 1.f;

  float barHeight = width * aspectRatio;
  drawTools.drawRect(0.f, yNdc - (barHeight * 0.5f), width, barHeight, false, glm::vec4(0.2f, 0.2f, 0.2f, 0.5f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
  drawTools.drawRect(0.f, yNdc - (barHeight * 0.5f), widthPercentage * width, barHeight, false, glm::vec4(0.f, 0.f, 0.8f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
}

//std::optional<std::string> imageForHud = "./res/textures/badhud.png";
std::optional<std::string> imageForHud = std::nullopt;

Component hudComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    if (imageForHud.has_value()){
      drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, imageForHud.value(), std::nullopt);
    }
    drawbar(drawTools, currentHealth);

    drawTools.drawText("health: " + std::to_string(static_cast<int>(currentHealth)), 0.85f, -0.9f, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt);
    drawTools.drawText(std::string("ammo: ") + std::to_string(ammoInfo.currentAmmo) + " / " + std::to_string(ammoInfo.totalAmmo), 0.85, -0.95, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt);

  	return BoundingBox2D {
  		.x = 0.f,
  		.y = 0.f,
  		.width = 2.f,
  		.height = 2.f,
  	};
  },
};
