#include "./terminal.h"

extern CustomApiBindings* gameapi;

struct Terminal {
  std::vector<std::vector<std::string>> pages;
};

Terminal testTerminal {
  .pages = {
    { 
      "Hello, welcome to the AfterWorld.  I bet you're wondering how you got here...",
      "Well",
      "Dont worry about that",
    },
    { "Well, this is the AfterWorld!..." },
    { "Stay tuned for updates!" },
  }
};


struct TerminalDisplayOptions {
  int pageIndex;
};



TerminalDisplayOptions terminalOptions {
  .pageIndex = 0,
};

Component terminalComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 0.8f), std::nullopt, true, std::nullopt, "../gameresources/build/terminals/terminal.png", std::nullopt);

    for (int i = 0; i < testTerminal.pages.at(terminalOptions.pageIndex).size(); i++){
      std::string& text = testTerminal.pages.at(terminalOptions.pageIndex).at(i);
      static float initialTime = gameapi -> timeSeconds(true);
      auto currIndex = static_cast<int>((gameapi -> timeSeconds(true) - initialTime) * 10.f);
      auto textSubtr = text.substr(0, currIndex);

      modlog("terminal", std::string("draw text: ") + std::to_string(i));

      drawCenteredTextReal(drawTools, textSubtr, 0.f, 0.4f + i * 0.1f, 0.04f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);
    }


  	return BoundingBox2D {
      .x = 0.f,
      .y = 0.f,
      .width = 2.f,
      .height = 2.f,
    };
  },
};
