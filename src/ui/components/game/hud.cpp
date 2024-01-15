#include "./hud.h"

AmmoHudInfo ammoInfo {
  .currentAmmo = 0,
  .totalAmmo = 0,
};
void setAmmoCount(AmmoHudInfo newAmmoInfo){
  ammoInfo = newAmmoInfo;
}

float currentHealth = 0.f;
void setHealth(float health){
  currentHealth = health;
}

void drawbar(DrawingTools& drawTools, float health, float yNdc){
  float width = 0.2f;
  float aspectRatio = 0.1f;
  float widthPercentage = glm::max(0.f, health / 100.f);
  drawTools.drawRect(-0.8f, yNdc, width + 0.01f, width * aspectRatio + 0.01f, false, glm::vec4(0.2f, 0.2f, 0.2f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
  drawTools.drawRect(-0.8f, yNdc, width, width * aspectRatio, false, glm::vec4(0.f, 0.f, 0.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
  drawTools.drawRect(-0.8f, yNdc, widthPercentage * width, width * aspectRatio, false, glm::vec4(0.f, 0.f, 0.8f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
}


Component hudComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, "./res/textures/badhud.png", std::nullopt);
    drawTools.drawText("health: " + std::to_string(static_cast<int>(currentHealth)), -0.9, 0.2, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt);

    drawbar(drawTools, currentHealth, 0.1f);

    drawTools.drawText(std::string("ammo: ") + std::to_string(ammoInfo.currentAmmo) + " / " + std::to_string(ammoInfo.totalAmmo), -0.9, 0.6, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt);

  	return BoundingBox2D {
  		.x = 0.f,
  		.y = 0.f,
  		.width = 2.f,
  		.height = 2.f,
  	};
  },
};
