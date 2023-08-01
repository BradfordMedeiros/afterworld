#ifndef MOD_AFTERWORLD_COMPONENTS_LIST_ITEM
#define MOD_AFTERWORLD_COMPONENTS_LIST_ITEM

#include "./common.h"

struct ImListItem {
  std::string value;
  std::optional<std::function<void()>> onClick;
  std::optional<objid> mappingId;
};

BoundingBox2D drawImMenuListItem(DrawingTools& drawTools, const ImListItem& menuItem, std::optional<objid> mappingId, float xoffset, float yoffset, float padding, std::optional<float> fontSizeStyle, float minwidth);
BoundingBox2D drawImMenuList(DrawingTools& drawTools, std::vector<ImListItem> list, std::optional<objid> mappingId, float xoffset, float yoffset2, float padding, std::optional<float> fontSizeStyle, float minwidth);
void processImMouseSelect(std::vector<ImListItem> list, std::optional<objid> mappingId);

#endif