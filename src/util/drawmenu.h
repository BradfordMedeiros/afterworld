#ifndef MOD_AFTERWORLD_UTIL_DRAWMENU
#define MOD_AFTERWORLD_UTIL_DRAWMENU

#include <string>
#include <vector>
#include "../../../ModEngine/src/cscript/cscript_binding.h"

struct MenuItemStyle {
  float margin;
  float padding;
  float minwidth;
  float xoffset;
  float yoffset;
  std::optional<glm::vec4> tint;
  std::optional<float> fontSizePerLetterNdi;
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
void processImMouseSelect(std::vector<NestedListItem> list, std::optional<objid> mappingId);

struct BoundingBox2D {
  float x;
  float y;
  float width;
  float height;
};

void drawDebugBoundingBox(BoundingBox2D& box);
std::string print(BoundingBox2D& box);

struct Props {
  //std::vector<Symbol, std::any> values;
  std::optional<objid> mappingId;
  float additionalYOffset;
  MenuItemStyle style;
};

BoundingBox2D drawImMenuListItem(const ImListItem& menuItem, std::optional<objid> mappingId, MenuItemStyle style, float additionalYOffset);
BoundingBox2D drawImMenuList(std::vector<ImListItem> list, std::optional<objid> mappingId, MenuItemStyle style, float additionalYOffset = 0.f);
BoundingBox2D drawImMenuList(std::vector<std::function<BoundingBox2D(Props&)>> list, std::optional<objid> mappingId, MenuItemStyle style, float additionalYOffset = 0.f);


void drawImNestedList(std::vector<NestedListItem> values, std::optional<objid> mappingId, MenuItemStyle style);



struct RadioButton {
  bool selected;
  std::optional<std::function<void()>> onClick;
  std::optional<objid> mappingId;
};

std::vector<RadioButton> createRadioButtons();
BoundingBox2D drawRadioButtons(std::vector<RadioButton> radioButtons, float xoffset, float yoffset, float width, float height);
void processImRadioMouseSelect(std::vector<RadioButton> radioButtons, std::optional<objid> mappingId);

struct ImGrid {
  int numCells;
};

void drawScreenspaceGrid(ImGrid grid);

#endif