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
  std::vector<PropPair> props;
};


PropPair* propPairAtIndex(std::vector<PropPair>& props, int symbol);
float floatFromProp(Props& props, int symbol, float defaultValue);
std::optional<float> floatFromProp(Props& props, int symbol);

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


struct UILevel {
  std::string name;
  std::string description;
  std::string image;
  std::string shortcut;
};


extern const int horizontalSymbol;
extern const int listItemsSymbol;
extern const int valueSymbol;
extern const int interfaceSymbol;
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
extern const int itemPaddingSymbol;
extern const int alignVertical;
extern const int flowHorizontal;
extern const int flowVertical;
extern const int titleSymbol;
extern const int detailSymbol;
extern const int xoffsetFromSymbol;
extern const int interpolationSymbol;
extern const int checkedSymbol;
extern const int editableSymbol;
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
extern const int borderColorSymbol;
extern const int barColorSymbol;
extern const int consoleInterfaceSymbol;
extern const int playLevelSymbol;
extern const int widthSymbol;
extern const int heightSymbol;
extern const int autofocusSymbol;

#endif