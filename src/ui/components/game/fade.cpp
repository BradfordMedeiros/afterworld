#include "./fade.h"

extern CustomApiBindings* gameapi;


LetterboxFade letterbox {
  .title = "",
  .animationDuration = 0.f,
  .animationHold = 0.f,
  .fadeOutDuration = 0.f,
  .boxColor = glm::vec4(0.f, 0.f, 0.f, 0.8f),
  .fadeColor = glm::vec4(0.1f, 0.1f, 0.1f, 0.0f),
  .fontSize = 0.02f,
};


void drawTitleBorders(DrawingTools& drawTools, float percentage, std::string& title){
  modlog("ui border", std::string("percentage is: ") + std::to_string(percentage));
  float barHeight = 0.2f * percentage;
  drawTools.drawRect(0.f, 1.f - (barHeight * 0.5f), 2.f, barHeight, false, letterbox.boxColor, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  drawTools.drawRect(0.f, -1.f + (barHeight * 0.5f), 2.f, barHeight, false, letterbox.boxColor, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  const float textPaddingRight = 0.04f;
  drawLeftText(drawTools, title, 1.f - textPaddingRight, -1.f + (barHeight * 0.5f), letterbox.fontSize, std::nullopt, std::nullopt);
}

void drawFadeAnimation(DrawingTools& drawTools, float percentage){
  drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(letterbox.fadeColor.x, letterbox.fadeColor.y, letterbox.fadeColor.z, letterbox.fadeColor.w * percentage), true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
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

void showLetterBox(std::string title, float duration){
  letterbox = LetterboxFade {
    .title = title,
    .animationDuration = duration * 0.25f,
    .animationHold = duration * 0.5f,
    .fadeOutDuration = duration * 0.25f ,
    .boxColor = glm::vec4(0.f, 0.f, 0.f, 0.8f),
    .fadeColor = glm::vec4(0.1f, 0.1f, 0.1f, 0.6f),
    .fontSize = 0.02f,
  };
  letterBoxStartTime = gameapi -> timeSeconds(false);
}


Component fadeComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto fade = calculateFade(letterbox, letterBoxStartTime);
    if (fade.has_value()){
      drawFadeAnimation(drawTools, fade.value());
      drawTitleBorders(drawTools, fade.value(), letterbox.title);
    }else {
      letterBoxStartTime = std::nullopt;
    }
    return BoundingBox2D {
      .x = 0.f, 
      .y = 0.f,
      .width = 2.f,
      .height = 2.f,
    };
  }
};