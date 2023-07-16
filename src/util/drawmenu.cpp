#include "./drawmenu.h"

extern CustomApiBindings* gameapi;

const float fontSizePerLetterNdi = 0.02f;


std::vector<MenuItem> calcMenuItems(std::vector<std::string>& elements, int mappingOffset, MenuItemStyle style){
  std::vector<MenuItem> menuItems = {};
  for (int i = 0; i < elements.size(); i++){
  	float textX = -0.9f + style.xoffset;
    auto level = elements.at(i);
    auto height = fontSizePerLetterNdi;
    auto width = level.size() * fontSizePerLetterNdi;
    auto left = textX;

    float minSpacingPerItem = height;
    float spacingPerItem = minSpacingPerItem + 2 * style.padding + 2 * style.margin;

    auto rectX = (left + (left + width)) / 2.f;
    auto rectY = 0.2 + (i * -1 * spacingPerItem);
    auto rectWidth = width + 2 * style.padding;
    auto rectHeight = height + 2 * style.padding;

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
    gameapi -> drawRect(menuItem.rectX, menuItem.rectY, menuItem.rectWidth, menuItem.rectHeight, false, tintVec.has_value() ? tintVec.value() : glm::vec4(0.f, 0.f, 0.f, 0.f), std::nullopt, true, menuItem.selectionId, std::nullopt);
    drawCenteredText(menuItem.text, menuItem.textX, menuItem.textY, fontSizePerLetterNdi, tint, menuItem.selectionId);
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


void drawImMenuList(std::vector<ImListItem> list, std::optional<objid> mappingId, MenuItemStyle style, int* _selectedIndex){
  std::vector<std::string> values;
  for (auto &item : list){
    values.push_back(item.value);
  }
  auto menuItems = calcMenuItems(values, 99999999, style);
  if (_selectedIndex != NULL){
    for (int i = 0; i < menuItems.size(); i++){
      bool menuItemSelected = mappingId.has_value() && menuItems.at(i).selectionId == mappingId.value();
      if (menuItemSelected){
        *_selectedIndex = i;
      }
    }
  }
  drawMenuItems(menuItems, mappingId, style.tint);
}

void processImMouseSelect(std::vector<ImListItem> list, std::optional<objid> mappingId){
  if (!mappingId.has_value()){
    return;
  }
  std::vector<std::string> values;
  for (auto &item : list){
    values.push_back(item.value);
  }
  auto menuItems = calcMenuItems(values, 99999999, MenuItemStyle { .margin = 0.1f, .padding = 0.1f, .xoffset = 0.5f, .tint = glm::vec4(1.f, 1.f, 1.f, 0.2f) });
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
  

void drawImNestedList(std::vector<NestedListItem> values, std::vector<objid> mappingIds, MenuItemStyle style){
  auto mappingId = mappingIds.size() > 0 ? mappingIds.at(0) : std::optional<objid>(std::nullopt);
  std::vector<ImListItem> items;
  for (auto &value : values){
    items.push_back(value.item);
  }

  int selectedIndex = -1;
  drawImMenuList(items, mappingId, style, &selectedIndex);
  if (selectedIndex != -1){
    std::cout << "nested list selected index: " << selectedIndex << std::endl;
  }

  if (selectedIndex == -1){
    return;
  }

  std::vector<ImListItem> items2;
  MenuItemStyle style2 = style;
  style2.xoffset += 0.25f;
  for (auto &value : values.at(selectedIndex).items){
    items2.push_back(value.item);
  }
  drawImMenuList(items2, mappingId, style2, NULL);

}