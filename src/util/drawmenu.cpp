#include "./drawmenu.h"

extern CustomApiBindings* gameapi;

const float fontSizePerLetterNdi = 0.02f;


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

void drawImMenuList(std::vector<ImListItem> list, std::optional<objid> mappingId, MenuItemStyle style, int mappingOffset){
  std::vector<std::string> values;
  for (auto &item : list){
    values.push_back(item.value);
  }

  for (int i = 0; i < list.size(); i++){
    ImListItem& menuItem = list.at(i);
    float textX = -0.9f + style.xoffset;
    auto level = menuItem.value;

    auto height = fontSizePerLetterNdi;
    auto width = level.size() * fontSizePerLetterNdi;
    auto left = textX;

    float minSpacingPerItem = height;
    float spacingPerItem = minSpacingPerItem + 2 * style.padding + 2 * style.margin;

    auto rectX = (left + (left + width)) / 2.f;
    auto rectY = 0.2 + (i * -1 * spacingPerItem);
    auto rectWidth = width + 2 * style.padding;
    auto rectHeight = height + 2 * style.padding;
    auto textY =  0.2 + (i * -1 * spacingPerItem);

    auto tint = (mappingId.has_value() && menuItem.mappingId.has_value() && menuItem.mappingId.value() == mappingId.value()) ? glm::vec4(1.f, 0.f, 0.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f);
    gameapi -> drawRect(rectX, rectY, rectWidth, rectHeight, false, style.tint, std::nullopt, true, menuItem.mappingId, std::nullopt);
    drawCenteredText(menuItem.value, textX, textY, fontSizePerLetterNdi, tint, menuItem.mappingId);
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

int transformMappingIds = 999999;
std::vector<ImListItem> imTransformMenu = {
  ImListItem { 
    .value = "translate", 
    .onClick = []() -> void { 
      std::cout << "translate" << std::endl; 
    },
    .mappingId = transformMappingIds++,
  },
  ImListItem { 
    .value = "scale", 
    .onClick = []() -> void { 
      std::cout << "scale" << std::endl; 
    },
    .mappingId = transformMappingIds++,
  },
  ImListItem { 
    .value = "rotate", 
    .onClick = []() -> void { 
      std::cout << "rotate" << std::endl; 
    },
    .mappingId = transformMappingIds++,
  }
};


std::vector<ImListItem> createPauseMenu(std::function<void()> resume, std::function<void()> goToMainMenu){
  int pauseMappingIds = 999999;
  std::vector<ImListItem> listItems;
  listItems.push_back(ImListItem {
    .value = "Resume",
    .onClick = resume,
    .mappingId = pauseMappingIds++,
  });
  listItems.push_back(ImListItem {
    .value = "Main Menu",
    .onClick = goToMainMenu,
    .mappingId = pauseMappingIds++,
  });
  return listItems;
}
  

int calculateMappingIndex(int initialMappingOffset, std::vector<NestedListItem> values, std::vector<int> path){
  return initialMappingOffset;
}
void drawImNestedList(std::vector<NestedListItem> values, std::vector<objid> mappingIds, MenuItemStyle style){
  std::vector<int> listOpenIndexs = { 1, 0 };

  //modassert(false, std::string("mapping id size: " ) + std::to_string(mappingIds.size()));
  std::vector<NestedListItem>* nestedListItems = &values;

  int mappingOffset = 999999;
  for (int i = -1; i < static_cast<int>(listOpenIndexs.size()); i++){
    std::vector<ImListItem> items;
    for (auto &value : *nestedListItems){
      items.push_back(value.item);
    }

    std::optional<int> mappingId = std::nullopt;
    drawImMenuList(items, mappingId, style, mappingOffset); // have a current path here, to calculation 
    mappingOffset += items.size();
    style.xoffset += 0.2f;
    if ((i + 1) < static_cast<int>(listOpenIndexs.size())){
      std::cout << "nested: " << (i + 1) << std::endl;
      nestedListItems = &(nestedListItems -> at(listOpenIndexs.at(i + 1)).items);
    }
  }
}