#ifndef MOD_AFTERWORLD_COMPONENTS_FADE
#define MOD_AFTERWORLD_COMPONENTS_FADE

#include "../common.h"
#include "../../../global.h"

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

extern Component fadeComponent;


#endif