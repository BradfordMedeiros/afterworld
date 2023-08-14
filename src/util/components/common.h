#ifndef MOD_AFTERWORLD_COMPONENTS_COMMON
#define MOD_AFTERWORLD_COMPONENTS_COMMON

#include <string>
#include <optional>
#include "../../util.h"

struct PropPair {
  int symbol;
  std::any value;
};

struct Props {
  //std::vector<Symbol, std::any> values;
  std::optional<objid> mappingId;
  std::vector<PropPair> props;
};

Props getDefaultProps(std::optional<objid> selectedId);

PropPair* propPairAtIndex(std::vector<PropPair>& props, int symbol);
int intFromProp(Props& props, int symbol, int defaultValue);
float floatFromProp(Props& props, int symbol, float defaultValue);
glm::vec3 vec3FromProp(Props& props, int symbol, glm::vec3 defaultValue);
glm::vec4 vec4FromProp(Props& props, int symbol, glm::vec4 defaultValue);
std::optional<std::function<void()>> fnFromProp(Props& props, int symbol);
std::string strFromProp(Props& props, int symbol, const char* defaultValue);

template <typename T>
T* typeFromProps(Props& props, int symbol){
  auto propPair = propPairAtIndex(props.props, symbol);
  if (!propPair){
    return NULL;
  }
  T* propValue = anycast<T>(propPair -> value);
  modassert(propValue, "invalid cast in typeFromProps");
  return propValue;
}


void updatePropValue(Props& props, int symbol, std::any value);

struct DrawingTools {
  std::function<void(std::string word, float left, float top, unsigned int fontSize, bool permatext, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<std::string> fontFamily, std::optional<objid> selectionId)> drawText;
  std::function<void(float centerX, float centerY, float width, float height, bool perma, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture)> drawRect;
  std::function<void(glm::vec3 fromPos, glm::vec3 toPos, bool perma, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture)> drawLine2D;
  std::function<void(objid, std::function<void()>)> registerCallbackFns;
};

struct BoundingBox2D {
  float x;
  float y;
  float width;
  float height;
};

struct Component {
  std::function<BoundingBox2D(DrawingTools&, Props&)> draw;
  std::function<void(std::optional<objid> mappingId, Props& props)> imMouseSelect;
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


Component withProps(Component& wrappedComponent, Props& props);
Component withPropsCopy(Component& wrappedComponent, Props props);

extern Component emptyComponent;

#endif