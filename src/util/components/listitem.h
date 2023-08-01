#ifndef MOD_AFTERWORLD_COMPONENTS_LIST_ITEM
#define MOD_AFTERWORLD_COMPONENTS_LIST_ITEM

#include "./common.h"

struct ImListItem {
  std::string value;
  std::optional<std::function<void()>> onClick;
  std::optional<objid> mappingId;
};

BoundingBox2D drawImMenuListItem(DrawingTools& drawTools, const ImListItem& menuItem, std::optional<objid> mappingId, MenuItemStyle style, float additionalYOffset);
BoundingBox2D drawImMenuListItem(DrawingTools& drawTools, const ImListItem& menuItem, std::optional<objid> mappingId, MenuItemStyle style, float additionalYOffset);
BoundingBox2D drawImMenuList(DrawingTools& drawTools, std::vector<ImListItem> list, std::optional<objid> mappingId, MenuItemStyle style, float additionalYOffset = 0.f);
void processImMouseSelect(std::vector<ImListItem> list, std::optional<objid> mappingId);

#endif