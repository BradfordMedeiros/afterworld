#include "./radiobutton.h"

BoundingBox2D drawRadioButtons(DrawingTools& drawTools, std::vector<RadioButton> radioButtons, float xoffset, float yoffset, float width, float height){
// drawTools.drawRect(rectX, rectY, rectWidth, rectHeight, false, style.tint, std::nullopt, true, menuItem.mappingId, std::nullopt);
  modassert(radioButtons.size() > 0, "need at least one radiobutton to render");
  const float spacing = 0.01f;

  std::optional<float> minX = std::nullopt;
  std::optional<float> maxX = std::nullopt;
  std::optional<float> minY = std::nullopt;
  std::optional<float> maxY = std::nullopt;

  for (int i = 0; i < radioButtons.size(); i++){
    RadioButton& radioButton = radioButtons.at(i);
    float x = xoffset + i * width + (i == 0 ? 0.f : (i * spacing));
    auto radioButtonColor = radioButton.hovered ? glm::vec4(0.f, 0.f, 1.f, 0.3f) : glm::vec4(0.f, 0.f, 0.f, 0.6f);
    if (radioButton.selected){
      radioButtonColor = glm::vec4(0.f, 0.f, 1.f, 0.6f);
    }
    drawTools.drawRect(x, yoffset, width, height, false, radioButtonColor, std::nullopt, true, radioButton.mappingId, std::nullopt);
    
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


Component createRadioButtonComponent(RadioButtonContainer& radioButtonContainer){
  Component radioButtonSelector  {
    .draw = [&radioButtonContainer](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
      for (int i = 0; i < radioButtonContainer.radioButtons.size(); i++){
         auto radioMappingId = radioButtonContainer.radioButtons.at(i).mappingId;
         auto selectedId = getGlobalState().selectedId;
         if (selectedId.has_value() && radioMappingId.has_value() && radioMappingId.value() == selectedId.value()){
           radioButtonContainer.radioButtons.at(i).hovered = true;
         }else{
           radioButtonContainer.radioButtons.at(i).hovered = false;
         }
      }
  
      auto boundingBox = drawRadioButtons(
        drawTools,
        radioButtonContainer.radioButtons,
        props.style.xoffset,
        props.style.yoffset,
        0.05f,
        0.15f
      );
      
      std::cout << "radio bounding box: " << print(boundingBox) << std::endl;
      drawDebugBoundingBox(drawTools, boundingBox);
      return boundingBox;
    },
    .imMouseSelect = [&radioButtonContainer](std::optional<objid> mappingIdSelected) -> void {
       for (int i = 0; i < radioButtonContainer.radioButtons.size(); i++){
         auto radioMappingId = radioButtonContainer.radioButtons.at(i).mappingId;
         if (mappingIdSelected.has_value() && radioMappingId.has_value() && radioMappingId.value() == mappingIdSelected.value()){
           radioButtonContainer.selectedRadioButtonIndex = i;
           if (radioButtonContainer.radioButtons.at(i).onClick.has_value()){
             radioButtonContainer.radioButtons.at(i).onClick.value()();
           }
           radioButtonContainer.radioButtons.at(i).selected = true;
         }else{
           radioButtonContainer.radioButtons.at(i).selected = false;
         }
       }
    }  
  };
  return radioButtonSelector;
}
