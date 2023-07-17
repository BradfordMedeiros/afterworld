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


struct MenuItemStyle {
  float margin;
  float padding;
  float xoffset;
  std::optional<glm::vec4> tint;
};


struct ImListItem {
  std::string value;
  std::optional<std::function<void()>> onClick;
  std::optional<objid> mappingId;
};

struct NestedListItem {
  ImListItem item;
  std::vector<NestedListItem> items;
};

void processImMouseSelect(std::vector<ImListItem> list, std::optional<objid> mappingId);
std::vector<ImListItem> createPauseMenu(std::function<void()> resume, std::function<void()> goToMainMenu);

extern std::vector<ImListItem> imTransformMenu;

void drawImMenuList(std::vector<ImListItem> list, std::optional<objid> mappingId, MenuItemStyle style, int mappingOffset = 99999999);
void drawImNestedList(std::vector<NestedListItem> values, std::vector<objid> mappingIds, MenuItemStyle style);

#endif