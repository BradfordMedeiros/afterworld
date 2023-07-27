#ifndef MOD_AFTERWORLD_COMPONENTS_NESTED_LIST
#define MOD_AFTERWORLD_COMPONENTS_NESTED_LIST

#include "./common.h"

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

BoundingBox2D drawImMenuListItem(const ImListItem& menuItem, std::optional<objid> mappingId, MenuItemStyle style, float additionalYOffset);
BoundingBox2D drawImMenuList(std::vector<ImListItem> list, std::optional<objid> mappingId, MenuItemStyle style, float additionalYOffset = 0.f);

Component createNestedList(std::vector<NestedListItem>& items);

#endif