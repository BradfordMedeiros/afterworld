#include "./dialog.h"



Component dialogComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto listItemPtr = typeFromProps<std::vector<ListComponentData>>(props, listItemsSymbol);
    modassert(listItemPtr, "invalid listItems prop");
 
    auto listItems = *listItemPtr;

    std::vector<Component> elements;

    auto titleValue = strFromProp(props, titleSymbol, "");
    if (titleValue != ""){
      auto titleTextbox = withPropsCopy(textbox, Props {
        .props = {
          PropPair { .symbol = valueSymbol, .value = titleValue },
        }
      });
      elements.push_back(titleTextbox);
    }


    auto detailValue = strFromProp(props, detailSymbol, "");
    if (detailValue != ""){
      auto detailTextbox = withPropsCopy(textbox, Props {
        .props = { 
          PropPair { .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 0.f, 0.f) },
          PropPair { .symbol = valueSymbol, .value = detailValue },
        },
      });
      elements.push_back(detailTextbox);      
    }

    auto textData = typeFromProps<TextData>(props, valueSymbol);
    modassert(textData, "text data is not defined dialog");
 
    auto onEdit = typeFromProps<std::function<void(TextData)>>(props, onInputSymbol);
    modassert(onEdit, "on edit is not defined dialog");

    Props textboxProps {
      .props = {
        PropPair { .symbol = editableSymbol, .value = true },
        PropPair { .symbol = textDataSymbol, .value = *textData },
        PropPair { .symbol = onInputSymbol, .value = *onEdit },
      }
    };
    auto textboxWithProps = withPropsCopy(textbox, textboxProps);
    elements.push_back(textboxWithProps);
    ////


    // option choices
    std::vector<Component> choiceElements;
    for (int i = 0 ; i < listItems.size(); i++){
      ListComponentData& listItemData = listItems.at(i);
      auto onClick = listItemData.onClick.has_value() ? listItemData.onClick.value() : []() -> void {};
      Props listItemProps {
        .props = {
          PropPair { .symbol = valueSymbol, .value = listItemData.name },
          PropPair { .symbol = onclickSymbol, .value = onClick },
          PropPair { .symbol = tintSymbol, .value = glm::vec4(0.2f, 0.2f, 0.2f, 0.8f) },
        },
      };
      auto listItemWithProps = withPropsCopy(listItem, listItemProps);
      choiceElements.push_back(listItemWithProps);
    }

    auto horizontalComponent = simpleHorizontalLayout(choiceElements);
    elements.push_back(horizontalComponent);

    auto listBoundingBox = simpleVerticalLayout(elements).draw(drawTools, props);

    //drawTools.drawText("some value", sides.right, sides.top, fontSize, false, std::nullopt, std::nullopt, true, std::nullopt, uniqueMenuItemMappingId());
    return listBoundingBox;
  },
};
