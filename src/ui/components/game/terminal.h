#ifndef MOD_AFTERWORLD_COMPONENTS_TERMINAL
#define MOD_AFTERWORLD_COMPONENTS_TERMINAL

#include "../common.h"
#include "../../../global.h"

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
struct TerminalConfig {
  float time;
	TerminalDisplayType terminalDisplay;
};

extern Component terminalComponent;

#endif

