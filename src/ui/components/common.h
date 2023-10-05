#ifndef MOD_AFTERWORLD_COMPONENTS_COMMON
#define MOD_AFTERWORLD_COMPONENTS_COMMON

#include <string>
#include <optional>
#include "../../util.h"

struct TrackedLocationData {
  glm::vec2 position;
  glm::vec2 size;
};
struct HandlerCallbackFn {
  TrackedLocationData trackedLocationData;
};
struct DrawingTools {
  std::function<void(std::string word, float left, float top, unsigned int fontSize, bool permatext, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<std::string> fontFamily, std::optional<objid> selectionId)> drawText;
  std::function<void(float centerX, float centerY, float width, float height, bool perma, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture, std::optional<objid> trackingId)> drawRect;
  std::function<void(glm::vec3 fromPos, glm::vec3 toPos, bool perma, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture)> drawLine2D;
  std::function<void(objid, std::function<void()>)> registerCallbackFns;
  std::function<void(objid, std::function<void(int)>)> registerCallbackRightFns;
  std::function<void(objid, std::function<void(int)>)> registerInputFns;
  std::optional<objid> selectedId;
  std::optional<objid> focusedId;
};

struct PropPair {
  int symbol;
  std::any value;
};

struct Props {
  std::vector<PropPair> props;
};

Props getDefaultProps();

PropPair* propPairAtIndex(std::vector<PropPair>& props, int symbol);
int intFromProp(Props& props, int symbol, int defaultValue);
float floatFromProp(Props& props, int symbol, float defaultValue);
glm::vec3 vec3FromProp(Props& props, int symbol, glm::vec3 defaultValue);
glm::vec4 vec4FromProp(Props& props, int symbol, glm::vec4 defaultValue);
std::optional<std::function<void()>> fnFromProp(Props& props, int symbol);
std::optional<std::function<void()>>* optFnFromProp(Props& props, int symbol);
std::optional<std::function<void(const char*)>> fnStrFromProp(Props& props, int symbol);
std::string strFromProp(Props& props, int symbol, const char* defaultValue);
objid objidFromProp(Props& props, int symbol);
bool boolFromProp(Props& prop, int symbol, bool defaultValue);

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


struct BoundingBox2D {
  float x;
  float y;
  float width;
  float height;
};

struct Component {
  std::function<BoundingBox2D(DrawingTools&, Props&)> draw;
};

void drawDebugBoundingBox(DrawingTools& drawTools, BoundingBox2D box, std::optional<glm::vec4> tint = std::nullopt);
void drawFillDebugBoundingBox(DrawingTools& drawTools, BoundingBox2D box, std::optional<glm::vec4> tint = glm::vec4(0.f, 0.f, 0.f, 0.2f));
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
void setBox(BoundingBoxMeasurer& box, float x, float y, float width, float height);
void measureBoundingBox(BoundingBoxMeasurer& boundingBoxMeasurer, BoundingBox2D& boundingBox);
BoundingBox2D measurerToBox(BoundingBoxMeasurer& box);

struct SideMeasurements {
  float left;
  float right;
  float top;
  float bottom;
};
SideMeasurements calculateSides(BoundingBox2D& elementsBox);
std::string print(SideMeasurements& sides);

Component withProps(Component& wrappedComponent, Props& props);
Component withPropsCopy(Component& wrappedComponent, Props props);

extern Component emptyComponent;

objid uniqueMenuItemMappingId();
void resetMenuItemMappingId();
void getMenuMappingData(int* _minId, int* _currentId);

void drawCenteredText(DrawingTools& drawTools, std::string text, float ndiOffsetX, float ndiOffsetY, float ndiSize, std::optional<glm::vec4> tint, std::optional<objid> selectionId);
void drawTextLeftHorzDownVert(DrawingTools& drawTools, std::string text, float ndiOffsetX, float ndiOffsetY, float ndiSize, std::optional<glm::vec4> tint, std::optional<objid> selectionId);
void drawWindowX(DrawingTools& drawTools, BoundingBox2D& boundingBox, std::function<void()>& onClickX);

enum DataStoreHint { NOHINT, VEC4, STRING };
void registerUiSource(int symbol, void* data, DataStoreHint typeHint = NOHINT);
void unregisterUiSource(int symbol);
void* uiConnect(int symbol);
std::string uiStoreToStr();

extern std::optional<std::function<void()>> nullClick;

extern const int horizontalSymbol;
extern const int listItemsSymbol;
extern const int valueSymbol;
extern const int onclickSymbol;
extern const int onclickRightSymbol;
extern const int onInputSymbol;
extern const int layoutSymbol;
extern const int routerSymbol;
extern const int tintSymbol;
extern const int minwidthSymbol;
extern const int minheightSymbol;
extern const int xoffsetSymbol;
extern const int yoffsetSymbol;
extern const int offsetSymbol;
extern const int elapsedTimeSymbol;
extern const int resumeSymbol;
extern const int goToMainMenuSymbol;
extern const int layoutSymbol;
extern const int sliderSymbol;
extern const int radioSymbol;
extern const int colorSymbol;
extern const int routerMappingSymbol;
extern const int paddingSymbol;
extern const int alignVertical;
extern const int flowHorizontal;
extern const int flowVertical;
extern const int titleSymbol;
extern const int detailSymbol;
extern const int xoffsetFromSymbol;
extern const int interpolationSymbol;
extern const int checkedSymbol;
extern const int editableSymbol;
extern const int textEditorDefault;
extern const int color;
extern const int onWindowDragSymbol;
extern const int onSlideSymbol;
extern const int enableSymbol;
extern const int dockTypeSymbol;
extern const int selectedSymbol;
extern const int fontsizeSymbol;
extern const int fixedSizeSymbol;
extern const int sizeSymbol;
extern const int limitSymbol;
extern const int focusTintSymbol;

#endif