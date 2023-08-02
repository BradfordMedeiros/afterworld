#include "./listitem.h"

const float fontSizePerLetterNdi = 0.02f;

void drawCenteredText(DrawingTools& drawTools, std::string text, float ndiOffsetX, float ndiOffsetY, float ndiSize, std::optional<glm::vec4> tint, std::optional<objid> selectionId){
  float fontSizeNdiEquivalent = ndiSize * 1000.f / 2.f;   // 1000 = 1 ndi
  drawTools.drawText(text, ndiOffsetX, ndiOffsetY, fontSizeNdiEquivalent, false, tint, std::nullopt, true, std::nullopt, selectionId);
}

BoundingBox2D drawImMenuListItem(DrawingTools& drawTools, const ImListItem& menuItem, std::optional<objid> mappingId, float xoffset, float yoffset, float padding, std::optional<float> fontSizeStyle, float minwidth, glm::vec4 rectTint, glm::vec4 color){
  float fontSize = fontSizeStyle.has_value() ? fontSizeStyle.value() : fontSizePerLetterNdi;
  auto height = fontSizePerLetterNdi;
  auto width = glm::max(menuItem.value.size() * fontSize, minwidth);

  auto rectX = (xoffset + (xoffset + width)) / 2.f;
  auto rectY = yoffset;
  auto rectWidth = width + 2 * padding;
  auto rectHeight = height + 2 * padding;
  auto textY = yoffset;

  auto tint = (mappingId.has_value() && menuItem.mappingId.has_value() && menuItem.mappingId.value() == mappingId.value()) ? glm::vec4(0.f, 0.f, 1.f, 1.f) : color;
  drawTools.drawRect(rectX, rectY, rectWidth, rectHeight, false, rectTint, std::nullopt, true, menuItem.mappingId, std::nullopt);
  drawCenteredText(drawTools, menuItem.value, xoffset, textY, fontSize, tint, menuItem.mappingId);
  return BoundingBox2D {
    .x = rectX,
    .y = rectY,
    .width = rectWidth,
    .height = rectHeight,
  };
}

BoundingBox2D drawImMenuList(DrawingTools& drawTools, std::vector<ImListItem> list, std::optional<objid> mappingId, float xoffset, float yoffset2, float padding, std::optional<float> fontSizeStyle, float minwidth, glm::vec4 tint, glm::vec4 color){
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

    auto boundingBox = drawImMenuListItem(drawTools, list.at(i), mappingId, xoffset, yoffset, padding, fontSizeStyle, minwidth, tint, color);
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

void processImMouseSelect(std::vector<ImListItem> list, std::optional<objid> mappingId){
  if (!mappingId.has_value()){
    return;
  }
  for (auto &item : list){
    if (item.mappingId.has_value() && (item.mappingId.value() == mappingId.value()) && item.onClick.has_value()){
      item.onClick.value()();
    }
  }
}


const int tintSymbol = getSymbol("tint");
const int colorSymbol = getSymbol("color");
Component createListItem(std::string value){
  ImListItem menuItem {
    .value = value,
    .onClick = []() -> void {
    },
    .mappingId = 0,
  };
  auto component = Component {
    .draw = [menuItem](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
        auto tint = vec4FromProp(props, tintSymbol, glm::vec4(1.f, 0.f, 0.f, 1.f));
        auto color = vec4FromProp(props, tintSymbol, glm::vec4(1.f, 1.f, 1.f, 1.f));
        std::cout << "tint is: " << print(tint) << std::endl;
        auto box = drawImMenuListItem(drawTools, menuItem, props.mappingId, props.style.xoffset, props.style.yoffset,  props.style.padding, props.style.fontSizePerLetterNdi, props.style.minwidth, tint, color);
        //auto yoffset = getProp<int>(props, symbolForName("yoffset"));
        drawDebugBoundingBox(drawTools, box);
        return box;
    },
    .imMouseSelect = [menuItem](std::optional<objid> mappingIdSelected) -> void {
      //if (mappingIdSelected.has_value() && mappingIdSelected.value() == menuItem.mappingId.value()){
      //  if (menuItem.onClick.has_value()){
      //    menuItem.onClick.value()();
      //  }
      //}
    },
  };
  return component;
}