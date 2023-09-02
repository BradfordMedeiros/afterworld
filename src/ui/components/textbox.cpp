#include "./textbox.h"

std::string insertString(std::string& str, int index, char character){
  auto prefix = str.substr(0, index);
  auto suffixIndex = index;
  if (suffixIndex >= str.size()){
    suffixIndex = str.size();
  }
  auto suffix = str.substr(suffixIndex, str.size());
  return prefix + character + suffix;
}

std::string valueText = "default textbox";
int cursorLocation = 2;

Component textbox {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto strValue = strFromProp(props, valueSymbol, "default textbox");

    auto isEditable = boolFromProp(props, editableSymbol, false);
    auto tint = vec4FromProp(props, tintSymbol, isEditable ? glm::vec4(1.f, 1.f, 1.f, 1.f) : glm::vec4(0.f, 0.f, 0.f, 1.f));
    auto color = vec4FromProp(props, colorSymbol, isEditable ? glm::vec4(0.f, 0.f, 0.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f));

    auto textValue = isEditable ? valueText : strValue;

    auto newTextValue = insertString(textValue, cursorLocation, 't');
    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol,   .value = newTextValue },
        PropPair { .symbol = paddingSymbol, .value = 0.02f },
        PropPair { .symbol = tintSymbol, .value = tint },
        PropPair { .symbol = colorSymbol, .value = color },
      },
    };

  	return listItem.draw(drawTools, listItemProps);
  },
};

