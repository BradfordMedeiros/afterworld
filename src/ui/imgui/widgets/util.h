#pragma once

#include "../../../../../ModEngine/src/ui/gui.h"
#include "../../../../src/ai/ai.h"
#include "../../../../src/gamecontrol/gametypes.h"
#include "../../../../src/gamecontrol/entity.h"
#include "./console_core.h"

void renderConsole(bool includePanel);

// This is a bit different not a widget, but this just sponors the drawing of the text
void onAlertFrame();
void pushAlertMessage(std::string message);


void renderAnimations(bool includePanel);
void renderGameType(bool includePanel);
void renderHitpoints(bool includePanel);
void renderInventory(bool includePanel);

void drawInputVisualization();


///
struct LetterboxFade {
  std::string title;
  std::optional<float> animationDuration;
  std::optional<float> animationHold;
  std::optional<float> fadeOutDuration;
  glm::vec4 boxColor;
  std::optional<glm::vec4> fadeColor;
  float fontSize;
};
void showLetterBox(std::string title, float duration);
void showLetterBoxHold(std::string title, float fadeInTime);
void hideLetterBox();

void drawFade();
