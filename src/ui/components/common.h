#ifndef MOD_AFTERWORLD_COMPONENTS_COMMON
#define MOD_AFTERWORLD_COMPONENTS_COMMON

#include <string>
#include <optional>
#include "../../util.h"

struct ImGrid {
  int numCells;
};
void drawScreenspaceGrid(ImGrid grid);


struct UILevel {
  std::string name;
  std::string description;
  std::string image;
  std::string shortcut;
};

#endif