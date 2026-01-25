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

void showLetterBoxDetail(std::string title, float fadeOutDuration, std::optional<glm::vec4> backgroundColor, glm::vec4 boxColor, float holdDuration, float fadeIn){
  letterbox = LetterboxFade {
    .title = title,
    .animationDuration = fadeIn,
    .animationHold = holdDuration,
    .fadeOutDuration = fadeOutDuration,
    .boxColor = boxColor,
    .fadeColor = backgroundColor,
    .fontSize = 0.02f,
  };
  letterBoxStartTime = gameapi -> timeSeconds(false);
}

void showLetterBox(std::string title, float duration){
  showLetterBoxDetail(title, duration * 0.25f, glm::vec4(0.f, 0.f, 0.f, 0.2f), glm::vec4(0.f, 0.f, 0.f, 0.8f), duration * 0.5f, duration * 0.25f);
}
void showLetterBoxHold(std::string title, float fadeInTime){
  showLetterBoxDetail(title, 0.f, std::nullopt, glm::vec4(0.f, 0.f, 0.f, 0.8f), 10000000.f, fadeInTime);
}
void hideLetterBox(){
  letterBoxStartTime = std::nullopt;
}

Component fadeComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto fade = calculateFade(letterbox, letterBoxStartTime);
    if (fade.has_value()){
      float percentage = fade.value();
      {
        // background
        if (letterbox.fadeColor.has_value()){
          drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(letterbox.fadeColor.value().x, letterbox.fadeColor.value().y, letterbox.fadeColor.value().z, letterbox.fadeColor.value().w * percentage), true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
        }
      }
      { // title borders
        modlog("ui border", std::string("percentage is: ") + std::to_string(percentage));
        float barHeight = 0.2f * percentage;
        drawTools.drawRect(0.f, 1.f - (barHeight * 0.5f), 2.f, barHeight, false, letterbox.boxColor, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
        drawTools.drawRect(0.f, -1.f + (barHeight * 0.5f), 2.f, barHeight, false, letterbox.boxColor, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
        const float textPaddingRight = 0.04f;
        drawLeftText(drawTools, letterbox.title, 1.f - textPaddingRight, -1.f + (barHeight * 0.5f), letterbox.fontSize, std::nullopt, std::nullopt);
      }
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