#include "./keyboard.h"

bool keyIsDown(char key);
bool leftMouseDown();
bool middleMouseDown();
bool rightMouseDown();

struct KeyLocation {
  char key;
  glm::vec2 topLeft;
  glm::vec2 bottomRight;
};

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
  KeyLocation {
    .key = 'A',
    .topLeft = glm::vec2(-0.720373, 0.017921),
    .bottomRight = glm::vec2(-0.612517, -0.230585),
  },
  KeyLocation {
    .key = 'S',
    .topLeft = glm::vec2(-0.583222, 0.003584),
    .bottomRight = glm::vec2(-0.475366, -0.235364),
  },
  KeyLocation {
    .key = 'D',
    .topLeft = glm::vec2(-0.444740, 0.017921),
    .bottomRight = glm::vec2(-0.339547, -0.230585),
  },
  KeyLocation {
    .key = 'F',
    .topLeft = glm::vec2(-0.315579, 0.020311),
    .bottomRight = glm::vec2(-0.211718, -0.230585),
  },
  KeyLocation {
    .key = 'G',
    .topLeft = glm::vec2(-0.183755, 0.025090),
    .bottomRight = glm::vec2(-0.073236, -0.235364),
  },
  KeyLocation {
    .key = 'H',
    .topLeft = glm::vec2(-0.049268, 0.025090),
    .bottomRight = glm::vec2(0.058589, -0.237754),
  },
  KeyLocation {
    .key = 'J',
    .topLeft = glm::vec2(0.081225, 0.034648),
    .bottomRight = glm::vec2(0.194407, -0.242533),
  },
  KeyLocation {
    .key = 'J',
    .topLeft = glm::vec2(0.083888, 0.032258),
    .bottomRight = glm::vec2(0.195739, -0.230585),
  },
  KeyLocation {
    .key = 'K',
    .topLeft = glm::vec2(0.217044, 0.027479),
    .bottomRight = glm::vec2(0.326232, -0.242533),
  },
  KeyLocation {
    .key = 'L',
    .topLeft = glm::vec2(0.356858, 0.027479),
    .bottomRight = glm::vec2(0.464714, -0.230585),
  },
  KeyLocation {
    .key = ';',
    .topLeft = glm::vec2(0.484687, 0.022700),
    .bottomRight = glm::vec2(0.591212, -0.240143),
  },
  KeyLocation {
    .key = '\'',
    .topLeft = glm::vec2(0.619174, 0.032258),
    .bottomRight = glm::vec2(0.727031, -0.240143),
  },
  //KeyLocation {
  //  .key = 13,  // enter
  //  .topLeft = glm::vec2(0.750999, 0.032258),
  //  .bottomRight = glm::vec2(0.958722, -0.230585),
  //},
  KeyLocation {
    .key = 'Z',
    .topLeft = glm::vec2(-0.654877, -0.300380),
    .bottomRight = glm::vec2(-0.542337, -0.561470),
  },
  KeyLocation {
    .key = 'X',
    .topLeft = glm::vec2(-0.521972, -0.292776),
    .bottomRight = glm::vec2(-0.405145, -0.566540),
  },
  KeyLocation {
    .key = 'C',
    .topLeft = glm::vec2(-0.384780, -0.292776),
    .bottomRight = glm::vec2(-0.273312, -0.564005),
  },
  KeyLocation {
    .key = 'V',
    .topLeft = glm::vec2(-0.249732, -0.295310),
    .bottomRight = glm::vec2(-0.140407, -0.564005),
  },
  KeyLocation {
    .key = 'B',
    .topLeft = glm::vec2(-0.116827, -0.300380),
    .bottomRight = glm::vec2(-0.008574, -0.564005),
  },
  KeyLocation {
    .key = 'N',
    .topLeft = glm::vec2(0.016077, -0.297845),
    .bottomRight = glm::vec2(0.128617, -0.566540),
  },
  KeyLocation {
    .key = 'M',
    .topLeft = glm::vec2(0.151125, -0.292776),
    .bottomRight = glm::vec2(0.261522, -0.564005),
  },
  KeyLocation {
    .key = ',',
    .topLeft = glm::vec2(0.287402, -0.298734),
    .bottomRight = glm::vec2(0.395013, -0.554430),
  },
  KeyLocation {
    .key = '.',
    .topLeft = glm::vec2(0.418635, -0.296203),
    .bottomRight = glm::vec2(0.531496, -0.556962),
  },
  KeyLocation {
    .key = '/',
    .topLeft = glm::vec2(0.551181, -0.286076),
    .bottomRight = glm::vec2(0.665354, -0.559494),
  },

  KeyLocation {
    .key = '7',
    .topLeft = glm::vec2(-0.016077, 0.673004),
    .bottomRight = glm::vec2(0.098607, 0.411914),
  },
  KeyLocation {
    .key = '8',
    .topLeft = glm::vec2(0.118971, 0.678074),
    .bottomRight = glm::vec2(0.227224, 0.404309),
  },
  KeyLocation {
    .key = '9',
    .topLeft = glm::vec2(0.251876, 0.673004),
    .bottomRight = glm::vec2(0.363344, 0.414449),
  },

  KeyLocation {
    .key = 'E',
    .topLeft = glm::vec2(-0.484444, 0.356052),
    .bottomRight = glm::vec2(-0.373968, 0.083431),
  },
  KeyLocation {
    .key = 'R',
    .topLeft = glm::vec2(-0.351111, 0.349001),
    .bottomRight = glm::vec2(-0.240635, 0.088132),
  },
  KeyLocation {
    .key = 'T',
    .topLeft = glm::vec2(-0.216508, 0.356052),
    .bottomRight = glm::vec2(-0.106032, 0.088132),
  },
  KeyLocation {
    .key = 'Y',
    .topLeft = glm::vec2(-0.081905, 0.349001),
    .bottomRight = glm::vec2(0.026032, 0.090482),
  },
  KeyLocation {
    .key = 'O',
    .topLeft = glm::vec2(0.318095, 0.353702),
    .bottomRight = glm::vec2(0.431111, 0.088132),
  },
  KeyLocation {
    .key = 'P',
    .topLeft = glm::vec2(0.452364f, 0.347291f),
    .bottomRight = glm::vec2(0.561044f, 0.0960591f),
  },
  KeyLocation {
    .key = '[',
    .topLeft = glm::vec2(0.584906, 0.355603),
    .bottomRight = glm::vec2(0.696934, 0.092672),
  },
  KeyLocation {
    .key = ']',
    .topLeft = glm::vec2(0.720519, 0.355603),
    .bottomRight = glm::vec2(0.826651, 0.090517),
  },
  KeyLocation {
    .key = '\\',
    .topLeft = glm::vec2(0.854953, 0.355603),
    .bottomRight = glm::vec2(0.966981, 0.088362),
  },
  


/*
fn keys
KeyLocation {
  .key = '-',
  .topLeft = glm::vec2(0.712755, 0.898606),
  .bottomRight = glm::vec2(0.824223, 0.733840),
},
KeyLocation {
  .key = ',',
  .topLeft = glm::vec2(0.572347, 0.901141),
  .bottomRight = glm::vec2(0.688103, 0.738910),
},
KeyLocation {
  .key = '+',
  .topLeft = glm::vec2(0.433012, 0.901141),
  .bottomRight = glm::vec2(0.546624, 0.738910),
},
KeyLocation {
  .key = '*',
  .topLeft = glm::vec2(0.301179, 0.891001),
  .bottomRight = glm::vec2(0.408360, 0.741445),
},
KeyLocation {
  .key = ')',
  .topLeft = glm::vec2(0.157556, 0.898606),
  .bottomRight = glm::vec2(0.270096, 0.728771),
},
KeyLocation {
  .key = '(',
  .topLeft = glm::vec2(0.019293, 0.898606),
  .bottomRight = glm::vec2(0.132905, 0.741445),
},
KeyLocation {
  .key = ''',
  .topLeft = glm::vec2(-0.122186, 0.898606),
  .bottomRight = glm::vec2(-0.007503, 0.738910),
},
KeyLocation {
  .key = '&',
  .topLeft = glm::vec2(-0.259378, 0.903676),
  .bottomRight = glm::vec2(-0.144695, 0.736375),
},
KeyLocation {
  .key = '%',
  .topLeft = glm::vec2(-0.399786, 0.896071),
  .bottomRight = glm::vec2(-0.284030, 0.733840),
},
KeyLocation {
  .key = '$',
  .topLeft = glm::vec2(-0.534834, 0.906210),
  .bottomRight = glm::vec2(-0.421222, 0.733840),
},
KeyLocation {
  .key = '#',
  .topLeft = glm::vec2(-0.558414, 0.835234),
  .bottomRight = glm::vec2(-0.586281, 0.728771),
},
KeyLocation {
  .key = '"',
  .topLeft = glm::vec2(-0.814577, 0.898606),
  .bottomRight = glm::vec2(-0.700965, 0.731305),
},
KeyLocation {
  .key = '',
  .topLeft = glm::vec2(-0.936763, 0.860583),
  .bottomRight = glm::vec2(-0.840300, 0.731305),
},

*/
  //KeyLocation {
  //  .key = GLFW_KEY_LEFT,
  //  .topLeft = glm::vec2(0.723473, -0.619772),
  //  .bottomRight = glm::vec2(0.831726, -0.759189),
  //},
  /*
  KeyLocation {
    .key = static_cast<char>(263),
    .topLeft = glm::vec2(0.717042, -0.776933),
    .bottomRight = glm::vec2(0.828510, -0.929024),
  },
  KeyLocation {
    .key = static_cast<char>(264),
    .topLeft = glm::vec2(0.640943, -0.860583),
    .bottomRight = glm::vec2(0.694534, -0.926489),
  },
  KeyLocation {
    .key = static_cast<char>(265),
    .topLeft = glm::vec2(0.853162, -0.784537),
    .bottomRight = glm::vec2(0.963558, -0.934094),
  },*/
  // fill out the rest of these
};

 


void drawKeyHighlighted(DrawingTools& drawTools, KeyLocation& key, glm::vec2 size){
  glm::vec2 topLeft = key.topLeft;
  glm::vec2 bottomRight = key.bottomRight;
  topLeft.x *= size.x;
  bottomRight.x *= size.x;
  topLeft.y *= size.y;
  bottomRight.y *= size.y;

  float height = topLeft.y - bottomRight.y;
  float width = bottomRight.x - topLeft.x;
  float midpointX = (topLeft.x + bottomRight.x) / 2.f;
  float midpointY = (bottomRight.y + topLeft.y) / 2.f;

  drawTools.drawRect(midpointX, midpointY, width, height, false, glm::vec4(0.f, 0.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
}

Component keyboardComponentInner {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    glm::vec2 size(0.5f, 0.5f);
    drawTools.drawRect(0.f, 0.f, size.x, size.y, false, glm::vec4(1.f, 1.f, 1.f, 0.8f), std::nullopt, true, std::nullopt, "../gameresources/build/misc/keyboard.png", std::nullopt);

    for (auto &key : keys){
      if (keyIsDown(key.key)){
        drawKeyHighlighted(drawTools, key, size * 0.5f); // 0.5f since this is using ndi
      }
    }

    return BoundingBox2D {
      .x = 0.f,
      .y = 0.f,
      .width = size.x,
      .height = size.y,
    };
  },
};

void drawMouseButtonValue(DrawingTools& drawTools, glm::vec2 size, float xoffset, bool selected){
  drawTools.drawRect(xoffset, 0.f, size.x, size.y, false, selected ? glm::vec4(0.f, 0.f, 1.f, 0.8f) : glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
}

Component mouseComponentInner {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    glm::vec2 size(0.05f, 0.05f);
      
    float totalWidth = size.x * 3;

    drawMouseButtonValue(drawTools, size, size.x * 0 + (size.x * 0.5f) - (totalWidth * 0.5f), leftMouseDown());
    drawMouseButtonValue(drawTools, size, size.x * 1 + (size.x * 0.5f) - (totalWidth * 0.5f), middleMouseDown());
    drawMouseButtonValue(drawTools, size, size.x * 2 + (size.x * 0.5f) - (totalWidth * 0.5f), rightMouseDown());

    return BoundingBox2D {
      .x = 0.f,
      .y = 0.f,
      .width = totalWidth,
      .height = size.y,
    };
  },
};

std::vector<Component> keyboardComponents {
  keyboardComponentInner,
  mouseComponentInner,
};

AlignmentParams keyboardAlignment {
  .layoutFlowHorizontal = UILayoutFlowPositive2,
  .layoutFlowVertical = UILayoutFlowPositive2,
};

Component keyboardComponent = simpleVerticalLayout(keyboardComponents, glm::vec2(0.f, 0.f), keyboardAlignment, glm::vec4(1.f, 1.f, 1.f, 1.f), 0.01f, glm::vec4(0.f, 0.f, 1.f, 0.2f));