#ifndef MOD_AFTERWORLD_UTIL_DRAWMENU
#define MOD_AFTERWORLD_UTIL_DRAWMENU

#include <string>
#include <vector>
#include "../../../ModEngine/src/cscript/cscript_binding.h"

struct MenuItem {
  std::string text;
  double rectX;
  double rectY;
  double rectWidth;
  double rectHeight;
  double textX;
  double textY;
  bool hovered;
};

std::vector<MenuItem> calcMenuItems(std::vector<std::string>& elements, float xNdc, float yNdc, float xoffset = 0.f);
void drawMenuItems(std::vector<MenuItem> items);

std::optional<int> highlightedMenuItem(std::vector<MenuItem>& items);

#endif