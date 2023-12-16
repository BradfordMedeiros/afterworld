#include "./listitem.h"

const float fontSizePerLetterNdi = 0.02f;


BoundingBox2D drawImMenuListItem(DrawingTools& drawTools, const ImListItem& menuItem, float xoffset, float yoffset, float padding, std::optional<float> fontSizeStyle, float minwidth, glm::vec4 rectTint, glm::vec4 color, std::function<void(int)>* inputFn){
  std::optional<objid> mappingId = drawTools.selectedId;
  float fontSize = fontSizeStyle.has_value() ? fontSizeStyle.value() : fontSizePerLetterNdi;

  float textWidth = 0.f;
  float height = 0.f;
  drawTools.getTextDimensionsNdi(menuItem.value, fontSize, true, std::nullopt, &textWidth, &height);

  auto width = glm::max(textWidth, minwidth);

  auto rectX = (xoffset + (xoffset + width)) / 2.f;
  auto rectY = yoffset;
  auto rectWidth = width + 2 * padding;
  auto rectHeight = height + 2 * padding;
  auto textY = yoffset;

  auto tint = (menuItem.onClick.has_value() && mappingId.has_value() && menuItem.mappingId.has_value() && menuItem.mappingId.value() == mappingId.value()) ? glm::vec4(0.f, 0.f, 1.f, 1.f) : color;

  if (menuItem.onClick.has_value()){
    drawTools.registerCallbackFns(menuItem.mappingId.value(), menuItem.onClick.value());
  }
  if (menuItem.onClick2.has_value()){
    drawTools.registerCallbackRightFns(menuItem.mappingId.value(), menuItem.onClick2.value());
  }

  if (inputFn){
    drawTools.registerInputFns(menuItem.mappingId.value(), *inputFn);
  }

  drawTools.drawRect(rectX, rectY, rectWidth, rectHeight, false, rectTint, std::nullopt, true, menuItem.mappingId, std::nullopt, std::nullopt);
  drawCenteredText(drawTools, menuItem.value, xoffset, textY, fontSize, tint, menuItem.mappingId);

  BoundingBox2D boundingBox {
    .x = rectX,
    .y = rectY,
    .width = rectWidth,
    .height = rectHeight,
  };
  drawDebugBoundingBox(drawTools, boundingBox, glm::vec4(1.f, 0.f, 0.f, 1.f));
  return boundingBox;
}

BoundingBox2D drawImMenuList(DrawingTools& drawTools, std::vector<ImListItem> list, float xoffset, float yoffset2, float padding, std::optional<float> fontSizeStyle, float minwidth, glm::vec4 tint, glm::vec4 color){
  std::optional<objid> mappingId = drawTools.selectedId;
  std::optional<float> minX = std::nullopt;
  std::optional<float> maxX = std::nullopt;

  std::optional<float> minY =  std::nullopt;
  std::optional<float> maxY = std::nullopt;


  float lastWidth = 0.f;
  float lastHeight = 0.f;
  float yoffset = yoffset2;

  modassert(list.size(), "draw im menu list - list is empty");
  for (int i = 0; i < list.size(); i++){
    ImListItem& menuItem = list.at(i);

    auto boundingBox = drawImMenuListItem(drawTools, list.at(i), xoffset, yoffset, padding, fontSizeStyle, minwidth, tint, color, NULL);
    float spacingPerItem = boundingBox.height;
    yoffset += -1 * spacingPerItem;

    lastWidth = boundingBox.width;
    lastHeight = boundingBox.height;

    float bottomY = boundingBox.y - (boundingBox.height * 0.5f);
    float topY = boundingBox.y + (boundingBox.height * 0.5f);
    float leftX = boundingBox.x - (boundingBox.width * 0.5f);
    float rightX = boundingBox.x + (boundingBox.width * 0.5f);

    if (!minX.has_value()){
      minX = leftX;
    }
    if (leftX < minX.value()){
      minX = leftX;
    }

    if (!maxX.has_value()){
      maxX = rightX;
    }
    if (rightX > maxX.value()){
      maxX = rightX;
    }

    if (!minY.has_value()){
      minY = bottomY;
    }
    if (bottomY < minY.value()){
      minY = bottomY;
    }

    if (!maxY.has_value()){
      maxY = topY;
    }
    if (topY > maxY.value()){
      maxY = topY;
    }
  }

  modassert(minX.has_value(), "minX does not have value");
  modassert(maxX.has_value(), "maxX does not have value");
  modassert(minY.has_value(), "minY does not have value");
  modassert(maxY.has_value(), "maxY does not have value");

  float width = maxX.value() - minX.value();
  float height = maxY.value() - minY.value();
  float centerX = minX.value() + (width * 0.5f);
  float centerY = minY.value() + (height * 0.5f);
  return BoundingBox2D {
    .x = centerX,
    .y = centerY,
    .width = width,
    .height = height,
  };
}

BoundingBox2D drawListItem(DrawingTools& drawTools, Props& props){
  auto id = uniqueMenuItemMappingId();

  auto strValue = strFromProp(props, valueSymbol, "");
  auto tint = vec4FromProp(props, tintSymbol, glm::vec4(1.f, 0.f, 0.f, 1.f));
  auto color = vec4FromProp(props, colorSymbol, glm::vec4(1.f, 1.f, 1.f, 1.f));
  auto minwidth = floatFromProp(props, minwidthSymbol, 0.f);
  float xoffset = floatFromProp(props, xoffsetSymbol, 0.f);
  float yoffset = floatFromProp(props, yoffsetSymbol, 0.f);
  auto onClick = fnFromProp(props, onclickSymbol);
  auto onClick2 = typeFromProps<std::function<void(int)>>(props, onclickRightSymbol);
  auto padding = floatFromProp(props, paddingSymbol, 0.0f);
  auto fontSize = floatFromProp(props, fontsizeSymbol, 0.015f);
  auto limit = intFromProp(props, limitSymbol, -1);
  auto focusTint = typeFromProps<glm::vec4>(props, focusTintSymbol);
  std::function<void(int)>* inputFnHandler = typeFromProps<std::function<void(int)>>(props, onInputSymbol);

  if (limit >= 0){
    strValue = strValue.substr(0, limit);
  }

  bool isFocused = drawTools.focusedId.has_value() && drawTools.focusedId.value() == id;
  ImListItem menuItem {
    .value = strValue,
    .onClick = onClick,
    .onClick2 = onClick2 ? (*onClick2) : std::optional<std::function<void(int)>>(std::nullopt),
    .mappingId = id,
  };
  auto box = drawImMenuListItem(drawTools, menuItem, xoffset, yoffset,  padding, fontSize, minwidth, tint, color, isFocused ? inputFnHandler : NULL);
  //auto yoffset = getProp<int>(props, symbolForName("yoffset"));
  if (focusTint && isFocused){
    drawDebugBoundingBox(drawTools, box, *focusTint);
  }
  return box;
}

Component listItem {
  .draw = drawListItem,
};
