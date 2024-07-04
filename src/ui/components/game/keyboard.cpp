#include "./keyboard.h"

bool keyIsDown(char key);

struct KeyLocation {
  char key;
  glm::vec2 topLeft;
  glm::vec2 bottomRight;
};

//;.basicRow({ 'q', 'w', 'e', 'r' });
std::vector<KeyLocation> keys {
  KeyLocation {
    .key = 'Q',
    .topLeft = glm::vec2(-0.750332f, 0.348781f),
    .bottomRight = glm::vec2(-0.648074f, 0.0926829f),
  },
  KeyLocation {
    .key = 'W',
    .topLeft = glm::vec2(-0.615228f, 0.343885f),
    .bottomRight = glm::vec2(-0.513256f, 0.0964029f),
  },
  KeyLocation {
    .key = 32,
    .topLeft = glm::vec2(-0.381457f, -0.626635f),
    .bottomRight = glm::vec2(0.255629f, -0.921522f),
  },

  // fill out the rest of these
   
  
};

 


void drawKeyHighlighted(DrawingTools& drawTools, KeyLocation& key){
  float height = key.topLeft.y - key.bottomRight.y;
  float width = key.bottomRight.x - key.topLeft.x;
  float midpointX = (key.topLeft.x + key.bottomRight.x) / 2.f;
  float midpointY = (key.bottomRight.y + key.topLeft.y) / 2.f;

  drawTools.drawRect(midpointX, midpointY, width, height, false, glm::vec4(0.f, 0.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
}

Component keyboardComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    const glm::vec4 backgroundColor(0.f, 0.f, 0.f, 0.6f);
    //drawTools.drawRect(-0.75, 0, 0.5f, 0.5f, false, glm::vec4(1.f, 1.f, 1.f, 0.8f), std::nullopt, true, std::nullopt, "../gameresources/build/misc/keyboard.png", std::nullopt);
    drawTools.drawRect(0, 0, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 0.8f), std::nullopt, true, std::nullopt, "../gameresources/build/misc/keyboard.png", std::nullopt);

    for (auto &key : keys){
      if (keyIsDown(key.key)){
        drawKeyHighlighted(drawTools, key);
      }
    }

    return BoundingBox2D {
      .x = -0.75f,
      .y = 0.f,
      .width = 0.5f,
      .height = 0.5f,
    };
  },
};
