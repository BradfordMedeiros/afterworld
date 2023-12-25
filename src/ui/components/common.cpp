#include "./common.h"

extern CustomApiBindings* gameapi;

std::string print(TrackedLocationData& data){
  return print(data.position) + ", "  + print(data.size);
}

void drawDebugBoundingBox(DrawingTools& drawTools, BoundingBox2D box, std::optional<glm::vec4> tint){
  //gameapi -> drawRect(box.x, box.y, box.width, box.height, false, glm::vec4(1.f, 0.f, 1.f, 0.2f), std::nullopt, true, std::nullopt, std::nullopt);
  //gameapi -> drawRect(-0.915000, -0.795, 0.17f, 0.15f, false, glm::vec4(1.f, 0.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
  float left = box.x - (box.width * 0.5f);
  float right = box.x + (box.width * 0.5f);
  float up = box.y + (box.height * 0.5f);
  float down = box.y - (box.height * 0.5f);
  drawTools.drawLine2D(glm::vec3(left, up, 0.f), glm::vec3(right, up, 0.f), false, tint, std::nullopt, true, std::nullopt, std::nullopt);
  drawTools.drawLine2D(glm::vec3(left, down, 0.f), glm::vec3(right, down, 0.f), false, tint, std::nullopt, true, std::nullopt, std::nullopt);
  drawTools.drawLine2D(glm::vec3(left, up, 0.f), glm::vec3(left, down, 0.f), false, tint, std::nullopt, true, std::nullopt, std::nullopt);
  drawTools.drawLine2D(glm::vec3(right, up, 0.f), glm::vec3(right, down, 0.f), false, tint, std::nullopt, true, std::nullopt, std::nullopt);
}

void drawFillDebugBoundingBox(DrawingTools& drawTools, BoundingBox2D box, std::optional<glm::vec4> tint){
  gameapi -> drawRect(box.x, box.y, box.width, box.height, false, tint, std::nullopt, true, std::nullopt, std::nullopt);
}

std::string print(BoundingBox2D& box){
  return std::string("x = " + std::to_string(box.x) + ", y = " + std::to_string(box.y) + ", width = " + std::to_string(box.width) + ", height = " + std::to_string(box.height));
}

void drawScreenspaceGrid(ImGrid grid){
  float numLines = grid.numCells - 1;
  float ndiSpacePerLine = 1.f / grid.numCells;

  for (int y = 0; y < numLines; y ++){
    float unitLineNdi = ndiSpacePerLine * (y + 1);
    float ndiY = (unitLineNdi * 2.f) - 1.f;
    gameapi -> drawLine2D(glm::vec3(-1.f, ndiY, 0.f), glm::vec3(1.f, ndiY, 0.f), false, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
    //modlog("drawscreenspace", std::string("draw line: - ") + std::to_string(unitLineNdi));
  }
  for (int x = 0; x < numLines; x ++){
    float unitLineNdi = ndiSpacePerLine * (x + 1);
    float ndiX = (unitLineNdi * 2.f) - 1.f;
    gameapi -> drawLine2D(glm::vec3(ndiX, -1.f, 0.f), glm::vec3(ndiX, 1.f, 0.f), false, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
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

void setBox(BoundingBoxMeasurer& box, float x, float y, float width, float height){
  float left = x - (width * 0.5f);
  float right = x + (width * 0.5f);
  float top = y + (height * 0.5f);
  float bottom = y - (height * 0.5f);
  setX(box, left);
  setX(box, right);
  setY(box, top);
  setY(box, bottom);
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
glm::vec3 vec3FromProp(PropPair& propPair){
  glm::vec3* vec3Value = anycast<glm::vec3>(propPair.value);
  modassert(vec3Value, "invalid prop");
  return *vec3Value;
}
glm::vec4 vec4FromProp(PropPair& propPair){
  glm::vec4* vec4Value = anycast<glm::vec4>(propPair.value);
  modassert(vec4Value, "invalid prop");
  return *vec4Value;
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

glm::vec3 vec3FromProp(Props& props, int symbol, glm::vec3 defaultValue){
  auto propPair = propPairAtIndex(props.props, symbol);
  if (propPair){
    return vec3FromProp(*propPair);
  }
  return defaultValue;
}
glm::vec4 vec4FromProp(Props& props, int symbol, glm::vec4 defaultValue){
  auto propPair = propPairAtIndex(props.props, symbol);
  if (propPair){
    return vec4FromProp(*propPair);
  }
  return defaultValue;
}

std::optional<std::function<void()>> fnFromProp(Props& props, int symbol){
  auto propPair = propPairAtIndex(props.props, symbol);
  if (propPair){
    const std::type_info& typeInfo = propPair -> value.type();
    //std::cout << "Type of std::any value: " << typeInfo.name() << std::endl;
    std::function<void()>* fnValue = anycast<std::function<void()>>(propPair -> value);
    modassert(fnValue, "fnFromProp invalid type");
    return *fnValue;

  }
  return std::nullopt;
}

std::optional<std::function<void()>>* optFnFromProp(Props& props, int symbol){
  auto propPair = propPairAtIndex(props.props, symbol);
  if (propPair){
    //const std::type_info& typeInfo = propPair -> value.type();
    //std::cout << "Type of std::any value: " << typeInfo.name() << std::endl;
    std::optional<std::function<void()>>* fnValue = anycast<std::optional<std::function<void()>>>(propPair -> value);
    return fnValue;
  }
  return NULL;
}

std::optional<std::function<void(const char*)>> fnStrFromProp(Props& props, int symbol){
  auto propPair = propPairAtIndex(props.props, symbol);
  if (propPair){
    const std::type_info& typeInfo = propPair -> value.type();
    //std::cout << "Type of std::any value: " << typeInfo.name() << std::endl;
    std::function<void(const char*)>* fnValue = anycast<std::function<void(const char*)>>(propPair -> value);
    modassert(fnValue, "fnFromProp invalid type");
    return *fnValue;
  }
  return std::nullopt;
}

std::string strFromProp(Props& props, int symbol, const char* defaultValue){
  auto strValue = typeFromProps<std::string>(props, symbol);
  if (!strValue){
    return defaultValue;
  }
  return *strValue;
}

objid objidFromProp(Props& props, int symbol){
  auto objIdValue = typeFromProps<objid>(props, symbol);
  modassert(objIdValue, "objid is null");
  return *objIdValue; 
}

bool boolFromProp(Props& props, int symbol, bool defaultValue){
  auto boolValue = typeFromProps<bool>(props, symbol);
  if (!boolValue){
    return defaultValue;
  }
  return *boolValue; 
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


Component withProps(Component& wrappedComponent, Props& outerProps){
  auto component = Component {
    .draw = [&wrappedComponent, &outerProps](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
      for (auto &prop : outerProps.props){
        updatePropValue(props, prop.symbol, prop.value);
      }
      return wrappedComponent.draw(drawTools, props);
    },
  };
  return component;
}

Component withPropsCopy(Component& wrappedComponent, Props outerProps){
  auto component = Component {
    .draw = [&wrappedComponent, outerProps](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
      Props outerPropsCopy = outerProps;
      for (auto &prop : props.props){
        updatePropValue(outerPropsCopy, prop.symbol, prop.value);
      }
      return wrappedComponent.draw(drawTools, outerPropsCopy);
    },
  };
  return component;
}



Props getDefaultProps(){
  return Props { 
    .props = {}
  };
}
 

Component emptyComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    return { .x = 0, .y = 0, .width = 0.f, .height = 0.f };
  },
};

int minManangedId = 990000;
objid uniqueMappingId  = minManangedId;
objid uniqueMenuItemMappingId(){
  uniqueMappingId++;
  return uniqueMappingId;
}
void resetMenuItemMappingId(){
  uniqueMappingId = minManangedId;
}

void getMenuMappingData(int* _minId, int* _currentId){
  *_minId = minManangedId;
  *_currentId = uniqueMappingId;
}


// this is actually down right aligned, but vert centered
void drawCenteredText(DrawingTools& drawTools, std::string text, float ndiOffsetX, float ndiOffsetY, float ndiSize, std::optional<glm::vec4> tint, std::optional<objid> selectionId){
  float fontSizeNdiEquivalent = ndiSize * 1000.f / 2.f;   // 1000 = 1 ndi
  drawTools.drawText(text, ndiOffsetX, ndiOffsetY, fontSizeNdiEquivalent, false, tint, std::nullopt, true, std::nullopt, selectionId);
}
void drawTextLeftHorzDownVert(DrawingTools& drawTools, std::string text, float ndiOffsetX, float ndiOffsetY, float ndiSize, std::optional<glm::vec4> tint, std::optional<objid> selectionId){
  float fontSizeNdiEquivalent = ndiSize * 1000.f / 2.f;   // 1000 = 1 ndi
  float width = text.size() * ndiSize;
  float height = text.size() * ndiSize;
  drawTools.drawText(text, ndiOffsetX - (width * 0.5f), ndiOffsetY - (height * 0.5f), fontSizeNdiEquivalent, false, tint, std::nullopt, true, std::nullopt, selectionId);
}

void drawWindowX(DrawingTools& drawTools, BoundingBox2D& boundingBox, std::function<void()>& onClickX){
  auto sides = calculateSides(boundingBox);
  auto xMappingId = uniqueMenuItemMappingId();
  bool xHovered =  drawTools.selectedId.has_value() && drawTools.selectedId.value() == xMappingId;
  drawTextLeftHorzDownVert(drawTools, "x", sides.right, sides.top, 0.04f, xHovered ? glm::vec4(1.f, 1.f, 1.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 0.4f), xMappingId);
  drawTools.registerCallbackFns(xMappingId, onClickX);
}

struct UiDataStore {
  std::map<int, void*> data;
  std::map<int, DataStoreHint> typeHints;
};

UiDataStore dataStore { .data = { }, .typeHints = {} };
void registerUiSource(int symbol, void* data, DataStoreHint typeHint){
  modassert(dataStore.data.find(symbol) == dataStore.data.end(), std::string("element already exists in data store: ") + std::to_string(symbol));
  dataStore.data[symbol] = data;
  dataStore.typeHints[symbol] = typeHint;
  modlog("uistore", std::string("registered data: ") + std::to_string(symbol));
}
void unregisterUiSource(int symbol){
  modlog("uistore", std::string("unregistered data: ") + std::to_string(symbol));
  dataStore.data.erase(symbol);
  dataStore.typeHints.erase(symbol);
}
void* uiConnect(int symbol){
  modlog("uistore", std::string("connected to: " ) + std::to_string(symbol));
  return dataStore.data.at(symbol);
}
struct UiStoreKeyValue {
  std::string key;
  std::string value;
};
std::vector<UiStoreKeyValue> printUiStoreDebug(){
  std::vector<UiStoreKeyValue> debugData;
  for (auto &[symbol, data] : dataStore.data){
    auto type = dataStore.typeHints.at(symbol);
    if (type == NOHINT){
      debugData.push_back(UiStoreKeyValue {
        .key = nameForSymbol(symbol),
        .value = "cannot serialize data",
      });
    }else if (type == VEC4){
      glm::vec4* value = static_cast<glm::vec4*>(data);
      debugData.push_back(UiStoreKeyValue {
        .key = nameForSymbol(symbol),
        .value = print(*value),
      });
    }else if (type == STRING){
      std::string* value = static_cast<std::string*>(data);
      debugData.push_back(UiStoreKeyValue {
        .key = nameForSymbol(symbol),
        .value = *value,
      });
    }else{
      modassert(false, "printUiStoreDebug invalid type hint");
    }
  }

  return debugData;
}

std::string uiStoreToStr(){
  std::string data = "";
  for (auto &dataValue : printUiStoreDebug()){
    data += dataValue.key + std::string("=") + dataValue.value + std::string("\n");
  }
  return data;
}

std::optional<std::function<void()>> nullClick = []() -> void {};


const int horizontalSymbol = getSymbol("horizontal");
const int listItemsSymbol = getSymbol("listitems");
const int valueSymbol = getSymbol("value");
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
const int textEditorDefault = getSymbol("text-editor-default");
const int color = getSymbol("color");
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