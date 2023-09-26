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


Component textbox {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    static TextData* textData = static_cast<TextData*>(uiConnect(textEditorDefault));

    auto strValue = strFromProp(props, valueSymbol, "default textbox");
    auto isEditable = boolFromProp(props, editableSymbol, false);
    auto tint = vec4FromProp(props, tintSymbol, isEditable ? glm::vec4(1.f, 1.f, 1.f, 1.f) : glm::vec4(0.f, 0.f, 0.f, 1.f));
    auto color = vec4FromProp(props, colorSymbol, isEditable ? glm::vec4(0.f, 0.f, 0.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f));
    auto onClick = fnFromProp(props, onclickSymbol);

    auto textValue = isEditable ? textData -> valueText : strValue;
    auto newTextValue = isEditable ?  insertString(textValue, textData -> cursorLocation, '|') : textValue;
    if (isEditable && textData -> highlightLength > 0 && textData -> cursorLocation != textValue.size()){
      newTextValue = insertString(newTextValue, textData -> cursorLocation + textData -> highlightLength + 1, '|');
    }
    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol,   .value = newTextValue },
        PropPair { .symbol = paddingSymbol, .value = 0.02f },
        PropPair { .symbol = tintSymbol, .value = tint },
        PropPair { .symbol = colorSymbol, .value = color },
      },
    };
    if (isEditable){
      listItemProps.props.push_back(PropPair { .symbol = focusTintSymbol, .value = glm::vec4(0.f, 0.f, 1.f, 1.f) });
    }
    if (onClick.has_value()){
      listItemProps.props.push_back(PropPair { .symbol = onclickSymbol, .value = onClick.value() });
    }
  	return listItem.draw(drawTools, listItemProps);
  },
};

