#ifndef MOD_AFTERWORLD_COMPONENTS_TEXTBOX
#define MOD_AFTERWORLD_COMPONENTS_TEXTBOX

#include "../common.h"
#include "./listitem.h"
#include "../../views/style.h"

extern Component textbox;
extern const int textDataSymbol;

struct TextData {
  std::string valueText;
  int cursorLocation;
  int highlightLength;
  int maxchars;
};

#endif