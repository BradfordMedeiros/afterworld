#ifndef MOD_AFTERWORLD_COMPONENTS_SELECT
#define MOD_AFTERWORLD_COMPONENTS_SELECT

#include "./listitem.h"
#include "./layout.h"

struct SelectOptions {
  std::function<std::vector<std::string>&()> getOptions;
  std::function<void(bool)> toggleExpanded;
  std::function<void(int, std::string&)> onSelect;
  std::function<int()> currentSelection;
  std::function<bool()> isExpanded;
};

extern Component selectComponent;

#endif