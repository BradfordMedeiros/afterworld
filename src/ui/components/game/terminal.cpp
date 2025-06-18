#include "./terminal.h"

extern CustomApiBindings* gameapi;

// Cantarell ?


glm::vec4 terminalFontColor(0.2745f, 0.5098f, 0.7059f, 1.f);

//TerminalDisplayType terminalDisplay = TerminalText {
//  .text = "<INCOMING MESSAGE>\n\nI have seen beyond this\n**********\n*********\nLorem\nipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Blandit cursus risus at ultrices mi tempus. Quam viverra orci sagittis eu volutpat odio. Non consectetur a erat nam at lectus. Sed tempus urna et pharetra pharetra massa. Eu volutpat odio facilisis mauris sit amet massa vitae tortor. Purus non enim praesent elementum facilisis leo vel. Tellus rutrum tellus pellentesque eu tincidunt tortor aliquam. Eu lobortis elementum nibh tellus molestie nunc non. Arcu dui vivamus arcu felis. Aliquam vestibulum morbi blandit cursus risus at ultrices mi. Urna et pharetra pharetra massa massa ultricies mi. Feugiat sed lectus vestibulum mattis ullamcorper. Senectus et netus et malesuada. Feugiat vivamus at augue eget arcu. Suspendisse sed nisi lacus sed viverra tellus. In nulla posuere sollicitudin aliquam ultrices sagittis orci. Vulputate eu scelerisque felis imperdiet proin fermentum leo vel. Tellus id interdum velit laoreet id donec ultrices tincidunt arcu.",
//};

Component terminalComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto terminalConfig = typeFromProps<TerminalConfig>(props, valueSymbol);
    modassert(terminalConfig, "terminalConfig must be defined for terminal");

    auto terminalImagePtr = std::get_if<TerminalImage>(&terminalConfig -> terminalDisplay);
    auto terminalImageLeftTextRightPtr = std::get_if<TerminalImageLeftTextRight>(&terminalConfig -> terminalDisplay);
    auto terminalTextPtr = std::get_if<TerminalText>(&terminalConfig -> terminalDisplay);
    modassert(terminalImagePtr || terminalImageLeftTextRightPtr || terminalTextPtr, "invalid terminal type");

    std::cout << "terminal time: " << terminalConfig -> time << std::endl;
    drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), true, std::nullopt, paths::TERMINAL_BACKGROUND, std::nullopt, std::nullopt);
    if (terminalImagePtr){
      drawTools.drawRect(0.f, 0.f, 1.f, 1.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), true, std::nullopt, terminalImagePtr -> image, std::nullopt, std::nullopt);
    }else if (terminalImageLeftTextRightPtr){
      drawTools.drawRect(-0.5f, 0.f, 1.f, 1.5f, false, glm::vec4(0.4f, 0.4f, 0.8f, 1.f), true, std::nullopt, terminalImageLeftTextRightPtr -> image, std::nullopt, std::nullopt);
      drawTools.drawRect(0.5f, 0.f, 1.f, 1.5f, false, glm::vec4(0.2f, 0.2f, 0.2f, 0.9f), true, std::nullopt, terminalImageLeftTextRightPtr -> image, std::nullopt, std::nullopt);
      auto currIndex = static_cast<int>((gameapi -> timeSeconds(false) - terminalConfig -> time) * 100.f);
      auto textSubtr = terminalImageLeftTextRightPtr -> text.substr(0, currIndex);
      drawRightText(drawTools, textSubtr, 0.f, 0.4f, 0.02f, terminalFontColor, std::nullopt, 1.f);
    }else if (terminalTextPtr){
      auto currIndex = static_cast<int>((gameapi -> timeSeconds(false) - terminalConfig -> time) * 100.f);
      auto textSubtr = terminalTextPtr -> text.substr(0, currIndex);
      drawTools.drawRect(-0.5f, 0.f, 1.f, 1.5f, false, glm::vec4(0.f, 0.f, 0.f, 0.7f), true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
      drawRightText(drawTools, textSubtr, -1.f, 0.4f, 0.02f, terminalFontColor, std::nullopt, 1.f);
    }
  	return BoundingBox2D {
      .x = 0.f,
      .y = 0.f,
      .width = 2.f,
      .height = 2.f,
    };
  },
};
