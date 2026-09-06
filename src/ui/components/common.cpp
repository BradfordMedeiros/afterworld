#include "./common.h"

extern CustomApiBindings* gameapi;


std::string print(BoundingBox2D& box){
  return std::string("x = " + std::to_string(box.x) + ", y = " + std::to_string(box.y) + ", width = " + std::to_string(box.width) + ", height = " + std::to_string(box.height));
}

void drawScreenspaceGrid(ImGrid grid){
  float numLines = grid.numCells - 1;
  float ndiSpacePerLine = 1.f / grid.numCells;

  for (int y = 0; y < numLines; y ++){
    float unitLineNdi = ndiSpacePerLine * (y + 1);
    float ndiY = (unitLineNdi * 2.f) - 1.f;
    gameapi -> drawLine2D(glm::vec3(-1.f, ndiY, 0.f), glm::vec3(1.f, ndiY, 0.f), false, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
    //modlog("drawscreenspace", std::string("draw line: - ") + std::to_string(unitLineNdi));
  }
  for (int x = 0; x < numLines; x ++){
    float unitLineNdi = ndiSpacePerLine * (x + 1);
    float ndiX = (unitLineNdi * 2.f) - 1.f;
    gameapi -> drawLine2D(glm::vec3(ndiX, -1.f, 0.f), glm::vec3(ndiX, 1.f, 0.f), false, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
    //modlog("drawscreenspace", std::string("draw line: - ") + std::to_string(unitLineNdi));
  }
}

BoundingBoxMeasurer createMeasurer(){
  return BoundingBoxMeasurer {
    .minX = std::nullopt,
    .maxX = std::nullopt,
    .minY = std::nullopt,
    .maxY = std::nullopt,
  };
}
void setX(BoundingBoxMeasurer& box, float value){
  if (!box.minX.has_value()){
    box.minX = value;
  }
  if (!box.maxX.has_value()){
    box.maxX = value;
  }
  if (value < box.minX.value()){
    box.minX = value;
  }
  if (value > box.maxX.value()){
    box.maxX = value;
  }
}
void setY(BoundingBoxMeasurer& box, float value){
  if (!box.minY.has_value()){
    box.minY = value;
  }
  if (!box.maxY.has_value()){
    box.maxY = value;
  }
  if (value < box.minY.value()){
    box.minY = value;
  }
  if (value > box.maxY.value()){
    box.maxY = value;
  }
}

void measureBoundingBox(BoundingBoxMeasurer& boundingBoxMeasurer, BoundingBox2D& boundingBox){
  setX(boundingBoxMeasurer, boundingBox.x + (boundingBox.width * 0.5f));
  setX(boundingBoxMeasurer, boundingBox.x - (boundingBox.width * 0.5f));
  setY(boundingBoxMeasurer, boundingBox.y + (boundingBox.height * 0.5f));
  setY(boundingBoxMeasurer, boundingBox.y - (boundingBox.height * 0.5f));
}
BoundingBox2D measurerToBox(BoundingBoxMeasurer& box){
  modassert(box.minX.has_value() && box.maxX.has_value() && box.minY.has_value() && box.maxY.has_value(), "box mins incomplete, probably no items");
  float minX = box.minX.value();
  float maxX = box.maxX.value();
  float minY = box.minY.value();
  float maxY = box.maxY.value();
  return BoundingBox2D {
    .x = (maxX + minX) * 0.5f,
    .y = (maxY + minY) * 0.5f,
    .width = maxX - minX,
    .height = maxY - minY,
  };
}

SideMeasurements calculateSides(BoundingBox2D& elementsBox){
  float elementsLeft = elementsBox.x - (elementsBox.width * 0.5f);
  float elementsRight = elementsBox.x + (elementsBox.width * 0.5f);
  float elementsTop = elementsBox.y + (elementsBox.height * 0.5f);
  float elementsBottom = elementsBox.y - (elementsBox.height * 0.5f);
  return SideMeasurements {
    .left = elementsLeft,
    .right = elementsRight,
    .top = elementsTop,
    .bottom = elementsBottom,
  };
}

std::string print(SideMeasurements& sides){
  return std::string("left = " + std::to_string(sides.left) + ", right = " + std::to_string(sides.right) + ", top = " + std::to_string(sides.top) + ", bottom = " + std::to_string(sides.bottom));
}

PropPair* propPairAtIndex(std::vector<PropPair>& props, int symbol){
  for (int i = 0; i < props.size(); i++){
    PropPair& propPair = props.at(i);
    if (propPair.symbol == symbol){
      return &propPair;
    }
  }
  return NULL;
}

int intFromProp(PropPair& propPair){
  int* intValue = anycast<int>(propPair.value);
  modassert(intValue, "invalid prop");
  return *intValue;
}
float floatFromProp(PropPair& propPair){
  float* floatValue = anycast<float>(propPair.value);
  modassert(floatValue, "invalid prop");
  return *floatValue;
}


int intFromProp(Props& props, int symbol, int defaultValue){
  auto propPair = propPairAtIndex(props.props, symbol);
  if (propPair){
    return intFromProp(*propPair);
  }
  return defaultValue;
}
float floatFromProp(Props& props, int symbol, float defaultValue){
  auto propPair = propPairAtIndex(props.props, symbol);
  if (propPair){
    return floatFromProp(*propPair);
  }
  return defaultValue;
}
std::optional<float> floatFromProp(Props& props, int symbol){
  auto propPair = propPairAtIndex(props.props, symbol);
  if (propPair){
    return floatFromProp(*propPair);
  }
  return std::nullopt;
}

void updatePropValue(Props& props, int symbol, std::any value){
  auto propPair = propPairAtIndex(props.props, symbol);
  if (propPair){
    propPair -> value = value;
    return;
  } 
  //modassert(false, std::string("prop does not exist: ") + nameForSymbol(symbol));
  props.props.push_back(PropPair {
    .symbol = symbol,
    .value = value,
  });
}



const int horizontalSymbol = getSymbol("horizontal");
const int listItemsSymbol = getSymbol("listitems");
const int valueSymbol = getSymbol("value");
const int interfaceSymbol = getSymbol("interface");
const int onclickSymbol = getSymbol("onclick");
const int onclickRightSymbol = getSymbol("onclick-right");
const int onInputSymbol = getSymbol("oninput");
const int layoutSymbol = getSymbol("layout");
const int routerSymbol = getSymbol("router");
const int tintSymbol = getSymbol("tint");
const int minwidthSymbol = getSymbol("minwidth");
const int minheightSymbol = getSymbol("minheight");
const int xoffsetSymbol = getSymbol("xoffset");
const int yoffsetSymbol = getSymbol("yoffset");
const int offsetSymbol = getSymbol("offset");
const int elapsedTimeSymbol = getSymbol("elapsedTime");
const int resumeSymbol = getSymbol("resume");
const int goToMainMenuSymbol = getSymbol("gotoMenu");
const int sliderSymbol = getSymbol("slider");
const int radioSymbol = getSymbol("radio");
const int colorSymbol = getSymbol("color");
const int routerMappingSymbol = getSymbol("router-mapping");
const int paddingSymbol = getSymbol("padding");
const int itemPaddingSymbol = getSymbol("item-padding");
const int alignVertical = getSymbol("align-vertical");
const int flowHorizontal = getSymbol("flow-horizontal");
const int flowVertical = getSymbol("flow-vertical");
const int titleSymbol = getSymbol("title");
const int detailSymbol = getSymbol("detail");
const int xoffsetFromSymbol =  getSymbol("xoffset-from");
const int interpolationSymbol = getSymbol("interpolation");
const int checkedSymbol = getSymbol("checked");
const int editableSymbol = getSymbol("editable");
const int onWindowDragSymbol = getSymbol("on-window-drag");
const int onSlideSymbol = getSymbol("on-slide");
const int enableSymbol = getSymbol("enable");
const int dockTypeSymbol = getSymbol("dock-type");
const int selectedSymbol = getSymbol("selected");
const int fontsizeSymbol = getSymbol("fontsize");
const int fixedSizeSymbol = getSymbol("fixed-size");
const int sizeSymbol = getSymbol("size");
const int limitSymbol = getSymbol("limit");
const int focusTintSymbol = getSymbol("focus-tint");
const int borderColorSymbol = getSymbol("border-color");
const int barColorSymbol = getSymbol("bar-color");
const int consoleInterfaceSymbol = getSymbol("console-interface");
const int playLevelSymbol = getSymbol("play-level");
const int widthSymbol = getSymbol("width");
const int heightSymbol = getSymbol("height");
const int autofocusSymbol = getSymbol("autofocus");
