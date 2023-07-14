#include "./drawmenu.h"

extern CustomApiBindings* gameapi;

const float fontSizePerLetterNdi = 0.02f;

std::vector<MenuItem> calcMenuItems(std::vector<std::string>& elements, float xoffset, int mappingOffset){
  std::vector<MenuItem> menuItems = {};
  for (int i = 0; i < elements.size(); i++){
  	float textX = -0.9f + xoffset;
    auto level = elements.at(i);
    auto height = fontSizePerLetterNdi;
    auto width = level.size() * fontSizePerLetterNdi;
    auto left = textX;

    float padding = 0.02f;
    float margin = 0.05f;
    float minSpacingPerItem = height;
    float spacingPerItem = minSpacingPerItem + 2 * padding + 2 * margin;

    auto rectX = (left + (left + width)) / 2.f;
    auto rectY = 0.2 + (i * -1 * spacingPerItem);
    auto rectWidth = width + 4 * padding;
    auto rectHeight = height + 4 * padding;

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

void drawMenuItems(std::vector<MenuItem> items, std::optional<objid> mappingId){
  for (auto &menuItem : items){
    auto tint =  (mappingId.has_value() && menuItem.selectionId == mappingId.value()) ? glm::vec4(1.f, 0.f, 0.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f);
    drawCenteredText(menuItem.text, menuItem.textX, menuItem.textY, fontSizePerLetterNdi, tint, std::nullopt);
    gameapi -> drawRect(menuItem.rectX, menuItem.rectY, menuItem.rectWidth, menuItem.rectHeight, false, glm::vec4(0.f, 0.f, 1.f, 0.f), std::nullopt, true, menuItem.selectionId, std::nullopt);
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