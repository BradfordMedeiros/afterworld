#ifndef MOD_AFTERWORLD_COMPONENTS_COMMON
#define MOD_AFTERWORLD_COMPONENTS_COMMON

#include <string>
#include <optional>
#include "../../util.h"

struct MenuItemStyle {
  float margin;
  float padding;
  float minwidth;
  float xoffset;
  float yoffset;
  std::optional<glm::vec4> tint;
  std::optional<float> fontSizePerLetterNdi;
};


struct Props {
  //std::vector<Symbol, std::any> values;
  std::optional<objid> mappingId;
  float additionalYOffset;
  MenuItemStyle style;
};

struct BoundingBox2D {
  float x;
  float y;
  float width;
  float height;
};

struct Component {
  std::function<BoundingBox2D(Props&)> draw;
  std::function<void(std::optional<objid> mappingId)> imMouseSelect;
};

void drawDebugBoundingBox(BoundingBox2D& box);
std::string print(BoundingBox2D& box);

struct ImGrid {
  int numCells;
};
void drawScreenspaceGrid(ImGrid grid);

#endif