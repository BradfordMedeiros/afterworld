#include "./terminal.h"

extern CustomApiBindings* gameapi;

std::optional<TerminalInterface> terminalInterface;  // TODO static state
bool shouldShowTerminal = false;
float lastToggleTerminalTime = 0.f;


void setShowTerminal(bool showTerminal){
  if (showTerminal){
    setTerminalConfig(terminalInterface.value().terminalConfig);
  }else{
    setTerminalConfig(std::nullopt);
  }
  if (showTerminal == shouldShowTerminal){
    return;
  }

  auto currTime = gameapi -> timeSeconds(false);
  if (currTime - lastToggleTerminalTime > 0.1f){
    lastToggleTerminalTime = currTime;
    shouldShowTerminal = showTerminal;
  }
}


bool isShowingTerminal(){
  return shouldShowTerminal;
}


std::unordered_map<std::string, std::vector<TerminalDisplayType>> terminals {
  { "test", {
    TerminalImage {
        .image = "../gameresources/build/textures/moonman.jpg",
    },
    TerminalImageLeftTextRight {
      .image = "../gameresources/build/textures/moonman.jpg",
      .text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Blandit cursus risus at ultrices mi tempus. Quam viverra orci sagittis eu volutpat odio. Non consectetur a erat nam at lectus. Sed tempus urna et pharetra pharetra massa. Eu volutpat odio facilisis mauris sit amet massa vitae tortor. Purus non enim praesent elementum facilisis leo vel. Tellus rutrum tellus pellentesque eu tincidunt tortor aliquam. Eu lobortis elementum nibh tellus molestie nunc non. Arcu dui vivamus arcu felis. Aliquam vestibulum morbi blandit cursus risus at ultrices mi. Urna et pharetra pharetra massa massa ultricies mi. Feugiat sed lectus vestibulum mattis ullamcorper. Senectus et netus et malesuada. Feugiat vivamus at augue eget arcu. Suspendisse sed nisi lacus sed viverra tellus. In nulla posuere sollicitudin aliquam ultrices sagittis orci. Vulputate eu scelerisque felis imperdiet proin fermentum leo vel. Tellus id interdum velit laoreet id donec ultrices tincidunt arcu.",
    },
    TerminalText {
      .text = "This is another page of text",
    }
  }}
};

TerminalConfig getTerminalConfig(std::string name, int pageIndex){
  return TerminalConfig {
    .time = gameapi -> timeSeconds(false),
    .terminalDisplay = terminals.at(name).at(pageIndex),
  };
}

void showTerminal(std::optional<std::string> name){
  if (!name.has_value()){
    terminalInterface = std::nullopt;
    setShowTerminal(false);
    return;
  }
  int pageIndex = 0;
  terminalInterface = TerminalInterface {
    .name = name.value(),
    .pageIndex = pageIndex,
    .terminalConfig = getTerminalConfig(name.value(), pageIndex),
  };
  setShowTerminal(true);
  *getTerminalConfig().value() = terminalInterface.value().terminalConfig; 
}
void nextTerminalPage(){
  if (terminalInterface.has_value()){
    TerminalInterface& terminal = terminalInterface.value();
    auto& terminalConfigs = terminals.at(terminalInterface.value().name);
    if (terminal.pageIndex < terminalConfigs.size() - 1){
      terminal.pageIndex++;
    }else{
      showTerminal(std::nullopt);
      return;
    }
    terminal.terminalConfig = getTerminalConfig(terminalInterface.value().name, terminal.pageIndex);
    *getTerminalConfig().value() = terminalInterface.value().terminalConfig; 
  }
}
void prevTerminalPage(){
  if (terminalInterface.has_value()){
    TerminalInterface& terminal = terminalInterface.value();
    auto& terminalConfigs = terminals.at(terminalInterface.value().name);
    if (terminal.pageIndex > 0){
      terminal.pageIndex--;
    }
    terminal.terminalConfig = getTerminalConfig(terminalInterface.value().name, terminal.pageIndex);
    *getTerminalConfig().value() = terminalInterface.value().terminalConfig; 
  }
}
