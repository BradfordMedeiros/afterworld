#ifndef MOD_AFTERWORLD_COMPONENTS_TEXTBOX
#define MOD_AFTERWORLD_COMPONENTS_TEXTBOX

#include "./common.h"
#include "./listitem.h"

extern Component textbox;

std::string insertString(std::string& str, int index, char character);

struct TextData {
  std::string valueText;
  int cursorLocation;
  int highlightLength;
  int maxchars;
};

#endif