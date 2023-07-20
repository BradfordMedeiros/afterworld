#include "./drawmenu.h"

extern CustomApiBindings* gameapi;

const float fontSizePerLetterNdi = 0.02f;


void drawCenteredText(std::string text, float ndiOffsetX, float ndiOffsetY, float ndiSize, std::optional<glm::vec4> tint, std::optional<objid> selectionId){
  float fontSizeNdiEquivalent = ndiSize * 1000.f / 2.f;   // 1000 = 1 ndi
  gameapi -> drawText(text, ndiOffsetX, ndiOffsetY, fontSizeNdiEquivalent, false, tint, std::nullopt, true, std::nullopt, selectionId);
}

BoundingBox2D drawImMenuList(std::vector<ImListItem> list, std::optional<objid> mappingId, MenuItemStyle style, float additionalYOffset){
  std::vector<std::string> values;
  for (auto &item : list){
    values.push_back(item.value);
  }

  std::optional<float> minX = std::nullopt;
  std::optional<float> maxX = std::nullopt;

  std::optional<float> minY  =  std::nullopt;
  std::optional<float> maxY = std::nullopt;

  modassert(list.size(), "draw im menu list - list is empty");
  for (int i = 0; i < list.size(); i++){
    ImListItem& menuItem = list.at(i);
    float textX = style.xoffset;
    auto level = menuItem.value;

    float fontSize = style.fontSizePerLetterNdi.has_value() ? style.fontSizePerLetterNdi.value() : fontSizePerLetterNdi;
    auto height = fontSizePerLetterNdi;
    auto width = glm::max(level.size() * fontSize, style.minwidth);
    auto left = textX;

    float minSpacingPerItem = height;
    float spacingPerItem = minSpacingPerItem + 2 * style.padding + 2 * style.margin;

    auto rectX = (left + (left + width)) / 2.f;
    auto rectY = style.yoffset + additionalYOffset + (i * -1 * spacingPerItem);
    auto rectWidth = width + 2 * style.padding;
    auto rectHeight = height + 2 * style.padding;
    auto textY = style.yoffset + additionalYOffset + (i * -1 * spacingPerItem);

    auto tint = (mappingId.has_value() && menuItem.mappingId.has_value() && menuItem.mappingId.value() == mappingId.value()) ? glm::vec4(1.f, 0.f, 0.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f);
    gameapi -> drawRect(rectX, rectY, rectWidth, rectHeight, false, style.tint, std::nullopt, true, menuItem.mappingId, std::nullopt);
    drawCenteredText(menuItem.value, textX, textY, fontSize, tint, menuItem.mappingId);

    float bottomY = rectY - (rectHeight * 0.5f);
    float topY = rectY + (rectHeight * 0.5f);
    float leftX = rectX - (rectWidth * 0.5f);
    float rightX = rectX + (rectWidth * 0.5f);

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

  
std::optional<std::vector<int>> searchNestedList(std::vector<NestedListItem>& values, objid mappingId, std::vector<int> currentPath){
  for (int i = 0; i < static_cast<int>(values.size()); i++){
    std::vector<int> nextPath = currentPath;
    nextPath.push_back(i);
    NestedListItem& nestedListItem = values.at(i);
    if (nestedListItem.item.mappingId.has_value() && nestedListItem.item.mappingId.value() == mappingId){
      return nextPath;
    }else{
      auto pathToFoundMapping = searchNestedList(nestedListItem.items, mappingId, nextPath);
      if (pathToFoundMapping.has_value()){
        return pathToFoundMapping;
      }
    }
  }
  return std::nullopt;
}
std::optional<std::vector<int>> calculateSelectedPath(std::vector<NestedListItem>& values, std::optional<objid> mappingId){
  if (!mappingId.has_value()){
    return {};
  }
  auto foundList = searchNestedList(values, mappingId.value(), {});
  return foundList;
}

void drawImNestedList(std::vector<NestedListItem> values, std::optional<objid> mappingId, MenuItemStyle style){
  std::vector<int> listOpenIndexs;
  auto selectedPath = calculateSelectedPath(values, mappingId);
  if (selectedPath.has_value()){
    listOpenIndexs = selectedPath.value();
  }
  std::vector<NestedListItem>* nestedListItems = &values;

  float additionalYOffset = 0.f;
  for (int i = -1; i < static_cast<int>(listOpenIndexs.size()); i++){
    if (nestedListItems -> size() == 0){
      continue;
    }
    std::vector<ImListItem> items;
    for (auto &value : *nestedListItems){
      items.push_back(value.item);
    }
    auto boundingBox = drawImMenuList(items, mappingId, style, additionalYOffset);
    float perElementHeight = boundingBox.height / items.size();
    style.xoffset += boundingBox.width;

    float multiplier = (i % 2) ? 1 : -1;
    style.tint = glm::vec4(style.tint.value().x + (multiplier * .2f), style.tint.value().y + (multiplier * .2f), style.tint.value().z + (multiplier * .2f), style.tint.value().w + (multiplier * .2f));
    if ((i + 1) < static_cast<int>(listOpenIndexs.size())){
      auto nextIndex = listOpenIndexs.at(i + 1);
      nestedListItems = &(nestedListItems -> at(nextIndex).items);
      additionalYOffset -=  (nextIndex * perElementHeight);
    }
  }
}

void processImMouseSelect(std::vector<ImListItem> list, std::optional<objid> mappingId){
  if (!mappingId.has_value()){
    return;
  }
  for (auto &item : list){
    if (item.mappingId.has_value() && item.mappingId.value() == mappingId.value() && item.onClick.has_value()){
      item.onClick.value()();
    }
  }
}

void processImMouseSelect(std::vector<NestedListItem> list, std::optional<objid> mappingId){
  if (!mappingId.has_value()){
    return;
  }
  std::vector<NestedListItem>* values = &list;
  auto path = calculateSelectedPath(list, mappingId);
  if (path.has_value()){
    for (auto index : path.value()){
      NestedListItem& item = values -> at(index);
      if (item.item.onClick.has_value()){
        item.item.onClick.value()();
      }
      values = &item.items;
    }
  }
}

void drawRadioButtons(std::vector<RadioButton> radioButtons){
//  gameapi -> drawRect(rectX, rectY, rectWidth, rectHeight, false, style.tint, std::nullopt, true, menuItem.mappingId, std::nullopt);
  float width = 0.02f;
  float height = 0.02f;
  float spacing = 0.01f;
  float xoffset = -0.5f;
  float yoffset = 0.f;
  for (int i = 0; i < radioButtons.size(); i++){
    RadioButton& radioButton = radioButtons.at(i);
    gameapi -> drawRect(xoffset + i * width + (i == 0 ? 0.f : (i * spacing)), yoffset, width, height, false, radioButton.selected? glm::vec4(0.f, 0.f, 1.f, 0.6f) : glm::vec4(0.f, 0.f, 0.f, 0.6f), std::nullopt, true, radioButton.mappingId, std::nullopt);
  }
}
void processImRadioMouseSelect(std::vector<RadioButton> radioButtons, std::optional<objid> mappingId){
  if (!mappingId.has_value()){
    return;
  }
  for (auto &radioButton : radioButtons){
    if (radioButton.mappingId.has_value() && radioButton.mappingId.value() == mappingId.value()){
      if(radioButton.onClick.has_value()){
        radioButton.onClick.value()();
      }
    }
  }
}
