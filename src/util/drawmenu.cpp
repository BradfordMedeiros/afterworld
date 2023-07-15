#include "./drawmenu.h"

extern CustomApiBindings* gameapi;

const float fontSizePerLetterNdi = 0.02f;

std::vector<MenuItem> calcMenuItems(std::vector<std::string>& elements, float xoffset, int mappingOffset, float padding, float margin){
  std::vector<MenuItem> menuItems = {};
  for (int i = 0; i < elements.size(); i++){
  	float textX = -0.9f + xoffset;
    auto level = elements.at(i);
    auto height = fontSizePerLetterNdi;
    auto width = level.size() * fontSizePerLetterNdi;
    auto left = textX;

    float minSpacingPerItem = height;
    float spacingPerItem = minSpacingPerItem + 2 * padding + 2 * margin;

    auto rectX = (left + (left + width)) / 2.f;
    auto rectY = 0.2 + (i * -1 * spacingPerItem);
    auto rectWidth = width + 2 * padding;
    auto rectHeight = height + 2 * padding;

    menuItems.push_back(MenuItem{
      .text = level,
      .rectX = rectX,
      .rectY = rectY,
      .rectWidth = rectWidth,
      .rectHeight = rectHeight,
      .textX = textX,
      .textY =  0.2 + (i * -1 * spacingPerItem),
      .selectionId = mappingOffset + i,
    });
  }
  return menuItems;  
}

void drawCenteredText(std::string text, float ndiOffsetX, float ndiOffsetY, float ndiSize, std::optional<glm::vec4> tint, std::optional<objid> selectionId){
  float fontSizeNdiEquivalent = ndiSize * 1000.f / 2.f;   // 1000 = 1 ndi
  gameapi -> drawText(text, ndiOffsetX, ndiOffsetY, fontSizeNdiEquivalent, false, tint, std::nullopt, true, std::nullopt, selectionId);
}

void drawMenuItems(std::vector<MenuItem> items, std::optional<objid> mappingId, std::optional<glm::vec4> tintVec){
  for (auto &menuItem : items){
    auto tint =  (mappingId.has_value() && menuItem.selectionId == mappingId.value()) ? glm::vec4(1.f, 0.f, 0.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f);
    drawCenteredText(menuItem.text, menuItem.textX, menuItem.textY, fontSizePerLetterNdi, tint, std::nullopt);
    gameapi -> drawRect(menuItem.rectX, menuItem.rectY, menuItem.rectWidth, menuItem.rectHeight, false, tintVec, std::nullopt, true, menuItem.selectionId, std::nullopt);
  }
}

std::optional<int> highlightedMenuItem(std::vector<MenuItem>& items, objid mappingId){
  for (int i = 0; i < items.size(); i++){
    auto item = items.at(i);
    if (item.selectionId == mappingId){
      return i;
    }
  }
  return std::nullopt;
}


void drawImMenuList(std::vector<ImListItem> list, std::optional<objid> mappingId, int xoffset){
  std::vector<std::string> values;
  for (auto &item : list){
    values.push_back(item.value);
  }
  auto menuItems = calcMenuItems(values, xoffset, 99999999, 0.1f, 0.1f);
  drawMenuItems(menuItems, mappingId, glm::vec4(0, 0, 1.f, 0.2f));
}

void processImMouseSelect(std::vector<ImListItem> list, std::optional<objid> mappingId){
  if (!mappingId.has_value()){
    return;
  }
  std::vector<std::string> values;
  for (auto &item : list){
    values.push_back(item.value);
  }
  auto menuItems = calcMenuItems(values, 0.5, 99999999, 0.1f, 0.1f);
  auto selectedItem = highlightedMenuItem(menuItems, mappingId.value());
  if (selectedItem.has_value()){
    auto onClick = list.at(selectedItem.value()).onClick;
    if (onClick.has_value()){
      onClick.value()();
    }
  }
}

std::vector<ImListItem> imTransformMenu = {
  ImListItem { 
    .value = "translate", 
    .onClick = []() -> void { std::cout << "translate" << std::endl; 
  }},
  ImListItem { 
    .value = "scale", 
    .onClick = []() -> void { std::cout << "scale" << std::endl; 
  }},
  ImListItem { 
    .value = "rotate", 
    .onClick = []() -> void { std::cout << "rotate" << std::endl; 
  }}
};



std::vector<ImListItem> createPauseMenu(std::function<void()> resume, std::function<void()> goToMainMenu){
  std::vector<ImListItem> listItems;
  listItems.push_back(ImListItem {
    .value = "Resume",
    .onClick = resume,
  });
  listItems.push_back(ImListItem {
    .value = "Main Menu",
    .onClick = goToMainMenu,
  });
  return listItems;
}
  

