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

const int textDataSymbol = getSymbol("textdata");


TextData deleteCharacter(TextData& textData){
  bool deleteSelection = textData.highlightLength > 0;
  auto index = deleteSelection ? (textData.cursorLocation + 1) : textData.cursorLocation;
  if (index == 0){
    return textData;
  }
  auto prefix = textData.valueText.substr(0, index -1);
  auto suffixIndex = index;
  if (deleteSelection){
    suffixIndex = suffixIndex + textData.highlightLength -1;
  }
  if (suffixIndex >= textData.valueText.size()){
    suffixIndex = textData.valueText.size();
  }
  auto suffix = textData.valueText.substr(suffixIndex, textData.valueText.size());
  auto newString = prefix + suffix;
  textData.valueText = newString;

  if (!deleteSelection){
    textData.cursorLocation--;
  }
  if (textData.cursorLocation < 0){
    textData.cursorLocation = 0;
  }
  textData.highlightLength = 0;
  return textData;
}
TextData insertCharacter(TextData& textData, char character){
  if (textData.highlightLength > 0){
    textData = deleteCharacter(textData);
  }
  auto newString = insertString(textData.valueText, textData.cursorLocation, character);
  textData.valueText = newString;
  textData.cursorLocation++;
  return textData;
}


Component textbox {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto textData = typeFromProps<TextData*>(props, textDataSymbol);
    auto strValue = strFromProp(props, valueSymbol, "default textbox");
    auto isEditable = boolFromProp(props, editableSymbol, false);
    auto tint = vec4FromProp(props, tintSymbol, isEditable ? glm::vec4(1.f, 1.f, 1.f, 1.f) : glm::vec4(0.f, 0.f, 0.f, 1.f));
    auto color = vec4FromProp(props, colorSymbol, isEditable ? glm::vec4(0.f, 0.f, 0.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f));
    auto onClick = fnFromProp(props, onclickSymbol);
    modassert(!isEditable || textData, "textbox editable but no text data provided");

    auto onEditTextPtr = typeFromProps<std::function<void(TextData)>>(props, onInputSymbol);

    auto textValue = isEditable ? (*textData) -> valueText : strValue;
    auto newTextValue = isEditable ?  insertString(textValue, (*textData) -> cursorLocation, '|') : textValue;
    if (isEditable && (*textData) -> highlightLength > 0 && (*textData) -> cursorLocation != textValue.size()){
      newTextValue = insertString(newTextValue, (*textData) -> cursorLocation + (*textData) -> highlightLength + 1, '|');
    }

    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol,   .value = newTextValue },
        PropPair { .symbol = paddingSymbol, .value = 0.02f },
        PropPair { .symbol = tintSymbol, .value = tint },
        PropPair { .symbol = colorSymbol, .value = color },
      },
    };
    if (isEditable && onEditTextPtr){
      auto onEditText = *onEditTextPtr;
      std::function<void(int)> onKeyPress = [onEditText, textData](int key) -> void {
        if (key == 263){        // left  key
          (*textData) -> cursorLocation--;
          if ((*textData) -> cursorLocation < 0){
            (*textData) -> cursorLocation = 0;
          }
          onEditText(**textData);
        }else if (key == 262){  // right key
          (*textData) -> cursorLocation++;
          if ((*textData) -> cursorLocation > (*textData) -> valueText.size()){
            (*textData) -> cursorLocation = (*textData) -> valueText.size();
          }
          onEditText(**textData);
        }else if (key == 265){
          // up key
          (*textData) -> highlightLength++;
          onEditText(**textData);
        }else if (key == 264){
          // downkey
          (*textData) -> highlightLength--;
          if ((*textData) -> highlightLength < 0){
            (*textData) -> highlightLength = 0;
          }
          onEditText(**textData);
        }else if (key == 257){
          onEditText(insertCharacter(**textData, '\n'));
        }else if (key == 261){
          // delete forward
        }else if (key == 259){
          onEditText(deleteCharacter(**textData));
        }else{
          onEditText(insertCharacter(**textData, key));
        }
      };
      listItemProps.props.push_back(PropPair { .symbol = onInputSymbol, onKeyPress });
    }

    if (isEditable){
      listItemProps.props.push_back(PropPair { .symbol = focusTintSymbol, .value = glm::vec4(0.f, 0.f, 1.f, 1.f) });
    }
    if (onClick.has_value()){
      listItemProps.props.push_back(PropPair { .symbol = onclickSymbol, .value = onClick.value() });
    }
  	return listItem.draw(drawTools, listItemProps);
  },
};

