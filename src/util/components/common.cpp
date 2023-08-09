#include "./common.h"

extern CustomApiBindings* gameapi;

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
    modlog("drawscreenspace", std::string("draw line: - ") + std::to_string(unitLineNdi));
  }
  for (int x = 0; x < numLines; x ++){
    float unitLineNdi = ndiSpacePerLine * (x + 1);
    float ndiX = (unitLineNdi * 2.f) - 1.f;
    gameapi -> drawLine2D(glm::vec3(ndiX, -1.f, 0.f), glm::vec3(ndiX, 1.f, 0.f), false, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
    modlog("drawscreenspace", std::string("draw line: - ") + std::to_string(unitLineNdi));
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
BoundingBox2D measurerToBox(BoundingBoxMeasurer& box){
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
    std::cout << "Type of std::any value: " << typeInfo.name() << std::endl;
    std::function<void()>* fnValue = anycast<std::function<void()>>(propPair -> value);
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
    .imMouseSelect = [&wrappedComponent, &outerProps](std::optional<objid> mappingIdSelected, Props& props) -> void {
      for (auto &prop : outerProps.props){
        updatePropValue(props, prop.symbol, prop.value);
      }
      wrappedComponent.imMouseSelect(mappingIdSelected, outerProps);
    },
  };
  return component;
}


