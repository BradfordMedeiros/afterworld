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
  objid selectionId;
};

std::vector<MenuItem> calcMenuItems(std::vector<std::string>& elements, float xoffset, int mappingOffset);
void drawMenuItems(std::vector<MenuItem> items, std::optional<objid> mappingId);

std::optional<int> highlightedMenuItem(std::vector<MenuItem>& items, objid mappingId);

#endif