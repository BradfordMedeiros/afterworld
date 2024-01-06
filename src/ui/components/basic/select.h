#ifndef MOD_AFTERWORLD_COMPONENTS_SELECT
#define MOD_AFTERWORLD_COMPONENTS_SELECT

#include "./listitem.h"
#include "./layout.h"

struct SelectOptions {
  std::vector<std::string> options;
  std::function<void(bool)> toggleExpanded;
  std::function<void(int, std::string&)> onSelect;
  int currentSelection;
  bool isExpanded;
};

extern Component selectComponent;

#endif