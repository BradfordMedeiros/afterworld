#include "./nestedlist.h"

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

BoundingBox2D drawImNestedList(DrawingTools& drawTools, std::vector<NestedListItem>& values , std::optional<objid> mappingId, MenuItemStyle style, float padding, float fontSizePerLetterNdi){
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

    auto boundingBox = drawImMenuList(drawTools, items, mappingId, style.xoffset, style.yoffset + additionalYOffset, padding, fontSizePerLetterNdi, style.minwidth);

    std::cout << "bounding box: " << print(boundingBox) << std::endl;
    // drawTools.drawRect(boundingBox.x, boundingBox.y, boundingBox.width + 0.01f, boundingBox.height + 0.01f, false, style.tint.value() + glm::vec4(0.3f, 0.3f, 0.3f, 0.f), std::nullopt, true, std::nullopt, std::nullopt);
    // drawTools.drawRect(0.f, 0.f, 0.2f, 0.2f, false, style.tint.value() + glm::vec4(0.3f, 0.3f, 0.3f, 0.f), std::nullopt, true, std::nullopt, std::nullopt);

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
  return BoundingBox2D {  // bounding box incorrect
    .x = 0.f,
    .y = 0.f,
    .width = 1.f,
    .height = 1.f,
  };
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


Component createNestedList(std::vector<NestedListItem>& items){
  Component component  {
    .draw = [&items](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
      float padding = 0.01f;
      float fontSizePerLetterNdi = 0.015f;
      return drawImNestedList(drawTools, items, props.mappingId, props.style, padding, fontSizePerLetterNdi);
    },
    .imMouseSelect = [&items](std::optional<objid> mappingIdSelected) -> void {
      processImMouseSelect(items, mappingIdSelected);
    }  
  };
  return component;
}
