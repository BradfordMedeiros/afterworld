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

struct DrawingTools {
  std::function<void(std::string word, float left, float top, unsigned int fontSize, bool permatext, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<std::string> fontFamily, std::optional<objid> selectionId)> drawText;
  std::function<void(float centerX, float centerY, float width, float height, bool perma, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture)> drawRect;
  std::function<void(glm::vec3 fromPos, glm::vec3 toPos, bool perma, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture)> drawLine2D;
};

struct BoundingBox2D {
  float x;
  float y;
  float width;
  float height;
};

struct Component {
  std::function<BoundingBox2D(DrawingTools&, Props&)> draw;
  std::function<void(std::optional<objid> mappingId)> imMouseSelect;
};



void drawDebugBoundingBox(DrawingTools& drawTools, BoundingBox2D box, std::optional<glm::vec4> tint = std::nullopt);
std::string print(BoundingBox2D& box);

struct ImGrid {
  int numCells;
};
void drawScreenspaceGrid(ImGrid grid);


struct BoundingBoxMeasurer {
  std::optional<float> minX;
  std::optional<float> maxX;
  std::optional<float> minY;
  std::optional<float> maxY;
};
BoundingBoxMeasurer createMeasurer();
void setX(BoundingBoxMeasurer& box, float value);
void setY(BoundingBoxMeasurer& box, float value);
BoundingBox2D measurerToBox(BoundingBoxMeasurer& box);

#endif