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
    auto textData = typeFromProps<TextData>(props, textDataSymbol);
    auto strValue = strFromProp(props, valueSymbol, "default textbox");
    auto isEditable = boolFromProp(props, editableSymbol, false);
    auto tint = vec4FromProp(props, tintSymbol, isEditable ? styles.highlightColor : glm::vec4(0.f, 0.f, 0.f, 0.f));
    auto color = vec4FromProp(props, colorSymbol, isEditable ? glm::vec4(0.f, 0.f, 0.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f));
    auto borderColor = typeFromProps<glm::vec4>(props, borderColorSymbol);
    auto onClick = fnFromProp(props, onclickSymbol);
    auto padding = floatFromProp(props, paddingSymbol, 0.02f);
    auto minwidth = typeFromProps<float>(props, minwidthSymbol);

    modassert(!isEditable || textData, "textbox editable but no text data provided");

    auto onEditTextPtr = typeFromProps<std::function<void(TextData)>>(props, onInputSymbol);

    auto textValue = isEditable ? textData -> valueText : strValue;
    auto newTextValue = isEditable ?  insertString(textValue, textData -> cursorLocation, '|') : textValue;
    if (isEditable && textData -> highlightLength > 0 && textData -> cursorLocation != textValue.size()){
      newTextValue = insertString(newTextValue, textData -> cursorLocation + textData -> highlightLength + 1, '|');
    }

    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol,   .value = newTextValue },
        PropPair { .symbol = paddingSymbol, .value = padding },
        PropPair { .symbol = tintSymbol, .value = tint },
        PropPair { .symbol = colorSymbol, .value = color },
      },
    };
    if (minwidth){
      listItemProps.props.push_back(PropPair { .symbol = minwidthSymbol, .value = *minwidth });
    }
    if (borderColor){
      listItemProps.props.push_back(PropPair { .symbol = borderColorSymbol, .value = *borderColor });
    }
    if (isEditable && onEditTextPtr){
      auto onEditText = *onEditTextPtr;

      TextData textDataValue2 = *textData;
      std::function<void(int)> onKeyPress = [onEditText, textDataValue2](int key) -> void {
        TextData textDataValue = textDataValue2;
        if (key == 263){        // left  key
          textDataValue.cursorLocation--;
          if (textDataValue.cursorLocation < 0){
            textDataValue.cursorLocation = 0;
          }
          onEditText(textDataValue);
        }else if (key == 262){  // right key
          textDataValue.cursorLocation++;
          if (textDataValue.cursorLocation > textDataValue.valueText.size()){
            textDataValue.cursorLocation = textDataValue.valueText.size();
          }
          onEditText(textDataValue);
        }else if (key == 265){
          // up key
          textDataValue.highlightLength++;
          onEditText(textDataValue);
        }else if (key == 264){
          // downkey
          textDataValue.highlightLength--;
          if (textDataValue.highlightLength < 0){
            textDataValue.highlightLength = 0;
          }
          onEditText(textDataValue);
        }else if (key == 257){
          onEditText(insertCharacter(textDataValue, '\n'));
        }else if (key == 261){
          // delete forward
        }else if (key == 259){
          onEditText(deleteCharacter(textDataValue));
        }else{
          onEditText(insertCharacter(textDataValue, key));
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

