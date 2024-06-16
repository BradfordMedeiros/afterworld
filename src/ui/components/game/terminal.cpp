#include "./terminal.h"

extern CustomApiBindings* gameapi;

// Cantarell ?

struct TerminalImage {
  std::string image;
};
struct TerminalImageLeftTextRight {
  std::string image;
  std::string text;
};
struct TerminalText {
  std::string text;
};

typedef std::variant<TerminalImage, TerminalImageLeftTextRight, TerminalText> TerminalDisplayType;

//TerminalDisplayType terminalDisplay = TerminalImage {
//  .image = "../gameresources/build/textures/moonman.jpg",
//};
//TerminalDisplayType terminalDisplay = TerminalImageLeftTextRight {
//  .image = "../gameresources/build/textures/moonman.jpg",
//  .text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Blandit cursus risus at ultrices mi tempus. Quam viverra orci sagittis eu volutpat odio. Non consectetur a erat nam at lectus. Sed tempus urna et pharetra pharetra massa. Eu volutpat odio facilisis mauris sit amet massa vitae tortor. Purus non enim praesent elementum facilisis leo vel. Tellus rutrum tellus pellentesque eu tincidunt tortor aliquam. Eu lobortis elementum nibh tellus molestie nunc non. Arcu dui vivamus arcu felis. Aliquam vestibulum morbi blandit cursus risus at ultrices mi. Urna et pharetra pharetra massa massa ultricies mi. Feugiat sed lectus vestibulum mattis ullamcorper. Senectus et netus et malesuada. Feugiat vivamus at augue eget arcu. Suspendisse sed nisi lacus sed viverra tellus. In nulla posuere sollicitudin aliquam ultrices sagittis orci. Vulputate eu scelerisque felis imperdiet proin fermentum leo vel. Tellus id interdum velit laoreet id donec ultrices tincidunt arcu.",
//};

glm::vec4 terminalFontColor(0.2745f, 0.5098f, 0.7059f, 1.f);

TerminalDisplayType terminalDisplay = TerminalText {
  .text = "<INCOMING MESSAGE>\n\nI have seen beyond this\n**********\n*********\nLorem\nipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Blandit cursus risus at ultrices mi tempus. Quam viverra orci sagittis eu volutpat odio. Non consectetur a erat nam at lectus. Sed tempus urna et pharetra pharetra massa. Eu volutpat odio facilisis mauris sit amet massa vitae tortor. Purus non enim praesent elementum facilisis leo vel. Tellus rutrum tellus pellentesque eu tincidunt tortor aliquam. Eu lobortis elementum nibh tellus molestie nunc non. Arcu dui vivamus arcu felis. Aliquam vestibulum morbi blandit cursus risus at ultrices mi. Urna et pharetra pharetra massa massa ultricies mi. Feugiat sed lectus vestibulum mattis ullamcorper. Senectus et netus et malesuada. Feugiat vivamus at augue eget arcu. Suspendisse sed nisi lacus sed viverra tellus. In nulla posuere sollicitudin aliquam ultrices sagittis orci. Vulputate eu scelerisque felis imperdiet proin fermentum leo vel. Tellus id interdum velit laoreet id donec ultrices tincidunt arcu.",
};

Component terminalComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {

    drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 0.99f), std::nullopt, true, std::nullopt, "../gameresources/build/terminals/terminal.png", std::nullopt);

    auto terminalImagePtr = std::get_if<TerminalImage>(&terminalDisplay);
    auto terminalImageLeftTextRightPtr = std::get_if<TerminalImageLeftTextRight>(&terminalDisplay);
    auto terminalTextPtr = std::get_if<TerminalText>(&terminalDisplay);
    modassert(terminalImagePtr || terminalImageLeftTextRightPtr || terminalTextPtr, "invalid terminal type");

    if (terminalImagePtr){
      drawTools.drawRect(0.f, 0.f, 1.f, 1.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, terminalImagePtr -> image, std::nullopt);
    }else if (terminalImageLeftTextRightPtr){
      drawTools.drawRect(-0.5f, 0.f, 1.f, 1.5f, false, glm::vec4(0.4f, 0.4f, 0.8f, 1.f), std::nullopt, true, std::nullopt, terminalImageLeftTextRightPtr -> image, std::nullopt);
      drawTools.drawRect(0.5f, 0.f, 1.f, 1.5f, false, glm::vec4(0.2f, 0.2f, 0.2f, 0.9f), std::nullopt, true, std::nullopt, terminalImageLeftTextRightPtr -> image, std::nullopt);
      drawRightText(drawTools, terminalImageLeftTextRightPtr -> text, 0.f, 0.4f, 0.02f, terminalFontColor, std::nullopt);
    }else if (terminalTextPtr){
      static float initialTime = gameapi -> timeSeconds(true);
      auto currIndex = static_cast<int>((gameapi -> timeSeconds(true) - initialTime) * 100.f);
      auto textSubtr = terminalTextPtr -> text.substr(0, currIndex);
      drawRightText(drawTools, textSubtr, -1.f, 0.4f, 0.02f, terminalFontColor, std::nullopt);
    }

    //modassert(terminalOptions.has_value(), "terminal options does not have a value");
    //TerminalDisplayOptions& terminalOpts = terminalOptions.value();
    //for (int i = 0; i < testTerminal.pages.at(terminalOpts.pageIndex).size(); i++){
    //  std::string& text = testTerminal.pages.at(terminalOpts.pageIndex).at(i);
    //  static float initialTime = gameapi -> timeSeconds(true);
    //  auto currIndex = static_cast<int>((gameapi -> timeSeconds(true) - initialTime) * 10.f);
    //  auto textSubtr = text.substr(0, currIndex);
    //  modlog("terminal", std::string("draw text: ") + std::to_string(i));
    //  drawCenteredTextReal(drawTools, textSubtr, 0.f, 0.4f + i * 0.1f, 0.04f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);
    //}


  	return BoundingBox2D {
      .x = 0.f,
      .y = 0.f,
      .width = 2.f,
      .height = 2.f,
    };
  },
};
