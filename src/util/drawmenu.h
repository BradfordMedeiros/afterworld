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

std::vector<MenuItem> calcMenuItems(std::vector<std::string>& elements, float xoffset, int mappingOffset, float padding = 0.02f, float margin = 0.05f);
void drawMenuItems(std::vector<MenuItem> items, std::optional<objid> mappingId, std::optional<glm::vec4> tint);

std::optional<int> highlightedMenuItem(std::vector<MenuItem>& items, objid mappingId);

struct ImListItem {
  std::string value;
  std::optional<std::function<void()>> onClick;
};
void drawImMenuList(std::vector<ImListItem> values, std::optional<objid> mappingId, int xoffset);
void processImMouseSelect(std::vector<ImListItem> list, std::optional<objid> mappingId);

std::vector<ImListItem> createPauseMenu(std::function<void()> resume, std::function<void()> goToMainMenu);

extern std::vector<ImListItem> imTransformMenu;

#endif