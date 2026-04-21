#ifndef MOD_AFTERWORLD_TERMINAL
#define MOD_AFTERWORLD_TERMINAL

#include "../ui/components/game/terminal.h"

struct TerminalInterface {
  std::string name;
  int pageIndex;
  TerminalConfig terminalConfig;
};

extern std::optional<TerminalInterface> terminalInterface;

void showTerminal(std::optional<std::string> name);
void nextTerminalPage();
void prevTerminalPage();
bool isShowingTerminal();

#endif 