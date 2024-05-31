#include "./hud.h"

extern CustomApiBindings* gameapi;

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

bool showActivate = false;
void setShowActivate(bool show){
  showActivate = show;
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

std::optional<float> letterBoxStartTime = std::nullopt;
const float LETTER_BOX_ANIMATION_DURATION = 5.f;
const float LETTER_BOX_TOTAL_DISPLAY_TIME = 10.f;
const glm::vec4 LETTER_BOX_COLOR(0.f, 0.f, 0.f, 0.8f);
const float LETTER_BOX_FONT_SIZE = 0.04f;

void showLetterBox(){
  letterBoxStartTime = gameapi -> timeSeconds(false);
}

void drawTitleBorders(DrawingTools& drawTools, float percentage, std::string&& title){
  modlog("ui border", std::string("percentage is: ") + std::to_string(percentage));
  float barHeight = 0.2f * percentage;
  drawTools.drawRect(0.f, 1.f - (barHeight * 0.5f), 2.f, barHeight, false, LETTER_BOX_COLOR, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
  drawTools.drawRect(0.f, -1.f + (barHeight * 0.5f), 2.f, barHeight, false, LETTER_BOX_COLOR, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
  //drawTools.drawText(title, 0.f, -1.f + (barHeight * 0.5f), LETTER_BOX_FONT_SIZE, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt);

  drawLeftText(drawTools, title, 1.f, -1.f + (barHeight * 0.5f), LETTER_BOX_FONT_SIZE, std::nullopt, std::nullopt);


}

void drawFadeAnimation(DrawingTools& drawTools, float percentage){
  drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(0.9f, 0.9f, 0.9f, 1.f * percentage), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
}


Component hudComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    static bool shouldShowLetterBox = true;
    if (shouldShowLetterBox){
      showLetterBox();
    }
    shouldShowLetterBox = false;

    if (imageForHud.has_value()){
      drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, imageForHud.value(), std::nullopt);
    }
    drawbar(drawTools, currentHealth);

    drawTools.drawText("health: " + std::to_string(static_cast<int>(currentHealth)), 0.85f, -0.9f, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt);
    drawTools.drawText(std::string("ammo: ") + std::to_string(ammoInfo.currentAmmo) + " / " + std::to_string(ammoInfo.totalAmmo), 0.85, -0.95, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt);

    if (showActivate){
      drawTools.drawText("press e to activate", 0.75f, 0.7f, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt);
    }

    if (letterBoxStartTime.has_value()){
      auto elapsedTime = gameapi -> timeSeconds(false) - letterBoxStartTime.value();
      float percentage = elapsedTime / LETTER_BOX_ANIMATION_DURATION;
      if (percentage > 1){
        percentage = 1.f;
      }
      if (elapsedTime > LETTER_BOX_TOTAL_DISPLAY_TIME){
        letterBoxStartTime = std::nullopt;
      }
      drawFadeAnimation(drawTools, percentage);
      drawTitleBorders(drawTools, percentage, "No Revelations");
    }

  	return BoundingBox2D {
  		.x = 0.f,
  		.y = 0.f,
  		.width = 2.f,
  		.height = 2.f,
  	};
  },
};
