#include "./radiobutton.h"

extern CustomApiBindings* gameapi;

std::vector<RadioButton> createRadioButtons(){
  int radioButtonMappingId = 342089;
  auto manipulatorMode = getStrWorldState("tools", "manipulator-mode").value();
  std::vector<RadioButton> radioButtonTest = {
    RadioButton { 
      .selected = manipulatorMode == "translate", 
      .onClick = getToggleWorldStateSetStr("tools", "manipulator-mode", "translate"),
      .mappingId = radioButtonMappingId++ 
    },
    RadioButton { 
      .selected = manipulatorMode == "scale",  
      .onClick = getToggleWorldStateSetStr("tools", "manipulator-mode", "scale"), 
      .mappingId = radioButtonMappingId++ 
    },
    RadioButton { 
      .selected = manipulatorMode == "rotate", 
      .onClick = getToggleWorldStateSetStr("tools", "manipulator-mode", "rotate"),
      .mappingId = radioButtonMappingId++ 
    },
  };
  return radioButtonTest;
}

BoundingBox2D drawRadioButtons(std::vector<RadioButton> radioButtons, float xoffset, float yoffset, float width, float height){
//  gameapi -> drawRect(rectX, rectY, rectWidth, rectHeight, false, style.tint, std::nullopt, true, menuItem.mappingId, std::nullopt);
  modassert(radioButtons.size() > 0, "need at least one radiobutton to render");
  const float spacing = 0.01f;

  std::optional<float> minX = std::nullopt;
  std::optional<float> maxX = std::nullopt;
  std::optional<float> minY = std::nullopt;
  std::optional<float> maxY = std::nullopt;
  for (int i = 0; i < radioButtons.size(); i++){
    RadioButton& radioButton = radioButtons.at(i);
    float x = xoffset + i * width + (i == 0 ? 0.f : (i * spacing));
    gameapi -> drawRect(x, yoffset, width, height, false, radioButton.selected? glm::vec4(0.f, 0.f, 1.f, 0.6f) : glm::vec4(0.f, 0.f, 0.f, 0.6f), std::nullopt, true, radioButton.mappingId, std::nullopt);
    
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;
    float left = x - halfWidth;
    float right = x + halfWidth;
    float top = yoffset + halfHeight;
    float bottom = yoffset - halfHeight;
    if (!minX.has_value()){
      minX = left;
    }
    if (!maxX.has_value()){
      maxX = right;
    }
    if (!minY.has_value()){
      minY = bottom;
    }
    if (!maxY.has_value()){
      maxY = top;
    }

    if (left < minX.value()){
      minX = left;
    }
    if (right > maxX.value()){
      maxX = right;
    }
    if (bottom < minY){
      minY = bottom;
    }
    if (top > maxY){
      maxY = top;
    }
  }

  std::cout << "radio minx: " << minX.value() << ", maxx: " << maxX.value() << std::endl;
  std::cout << "radio miny: " << minY.value() << ", maxy: " << maxY.value() << std::endl;

  return BoundingBox2D {
    .x = (maxX.value() + minX.value()) * 0.5f,
    .y = (maxY.value() + minY.value()) * 0.5f,
    .width = maxX.value() - minX.value(),
    .height = maxY.value() - minY.value(),
  };
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
