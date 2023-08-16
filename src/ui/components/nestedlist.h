#ifndef MOD_AFTERWORLD_COMPONENTS_NESTED_LIST
#define MOD_AFTERWORLD_COMPONENTS_NESTED_LIST

#include "./common.h"
#include "./listitem.h"


struct NestedListItem {
  ImListItem item;
  std::vector<NestedListItem> items;
};

extern Component nestedList;

#endif