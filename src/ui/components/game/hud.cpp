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


struct LetterboxFade {
  std::optional<float> animationDuration;
  std::optional<float> animationHold;
  std::optional<float> fadeOutDuration;
  glm::vec4 boxColor;
  glm::vec4 fadeColor;
  float fontSize;
};

LetterboxFade letterbox {
  .animationDuration = 2.f,
  .animationHold = 4.f,
  .fadeOutDuration = 2.f,
  .boxColor = glm::vec4(0.f, 0.f, 0.f, 0.8f),
  .fadeColor = glm::vec4(0.1f, 0.1f, 0.1f, 0.6f),
  .fontSize = 0.02f,
};


void drawTitleBorders(DrawingTools& drawTools, float percentage, std::string& title){
  modlog("ui border", std::string("percentage is: ") + std::to_string(percentage));
  float barHeight = 0.2f * percentage;
  drawTools.drawRect(0.f, 1.f - (barHeight * 0.5f), 2.f, barHeight, false, letterbox.boxColor, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
  drawTools.drawRect(0.f, -1.f + (barHeight * 0.5f), 2.f, barHeight, false, letterbox.boxColor, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
  const float textPaddingRight = 0.04f;
  drawLeftText(drawTools, title, 1.f - textPaddingRight, -1.f + (barHeight * 0.5f), letterbox.fontSize, std::nullopt, std::nullopt);
}

void drawFadeAnimation(DrawingTools& drawTools, float percentage){
  drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(letterbox.fadeColor.x, letterbox.fadeColor.y, letterbox.fadeColor.z, letterbox.fadeColor.w * percentage), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
}

struct FadeResult {
  float barPercentage;
  float fadePercentage;
};

std::optional<float> calculateFade(LetterboxFade& fade, std::optional<float> letterBoxStartTime){
  if (!letterBoxStartTime.has_value()){
    return std::nullopt;
  }
  float elapsedTime = gameapi -> timeSeconds(false) - letterBoxStartTime.value();
  float holdStart = (letterbox.animationDuration.has_value() ? letterbox.animationDuration.value() : 0.f);
  float fadeOutStart = holdStart + (letterbox.animationHold.has_value() ? letterbox.animationHold.value() : 0.f);
  float fadeEnd = fadeOutStart + (letterbox.fadeOutDuration.has_value() ? letterbox.fadeOutDuration.value() : 0.f);

  if (elapsedTime < holdStart){
    if (!letterbox.animationDuration.has_value()){
      return 1.f;
    }
    float percentage = elapsedTime / letterbox.animationDuration.value();
    return percentage;
  }else if (elapsedTime >= holdStart && elapsedTime < fadeOutStart){
    return 1.f;
  }else if (elapsedTime < fadeEnd) {
    if (!letterbox.fadeOutDuration.has_value()){
      return 0.f;
    }
    float percentage = (elapsedTime - fadeOutStart) / letterbox.animationDuration.value();
    return (1.f - percentage);
  }

  return std::nullopt;
}

std::optional<float> letterBoxStartTime = std::nullopt;
std::string title = "no Revelations";
void showLetterBox(){
  letterBoxStartTime = gameapi -> timeSeconds(false);
}


Component hudComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    static bool shouldShowLetterBox = false;
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

    auto fade = calculateFade(letterbox, letterBoxStartTime);
    if (fade.has_value()){
      drawFadeAnimation(drawTools, fade.value());
      drawTitleBorders(drawTools, fade.value(), title);
    }else {
      letterBoxStartTime = std::nullopt;
    }


  	return BoundingBox2D {
  		.x = 0.f,
  		.y = 0.f,
  		.width = 2.f,
  		.height = 2.f,
  	};
  },
};
