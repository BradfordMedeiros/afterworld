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


std::vector<ImListItem> createPauseMenu(std::function<void()> resume, std::function<void()> goToMainMenu);

extern std::vector<ImListItem> imTransformMenu;

struct BoundingBox2D {
  float x;
  float y;
  float width;
  float height;
};

BoundingBox2D drawImMenuList(std::vector<ImListItem> list, std::optional<objid> mappingId, MenuItemStyle style, float additionalYOffset = 0.f);
void drawImNestedList(std::vector<NestedListItem> values, std::optional<objid> mappingId, MenuItemStyle style);

#endif