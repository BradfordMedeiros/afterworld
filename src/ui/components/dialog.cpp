#include "./dialog.h"



Component dialogComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto listItemPtr = typeFromProps<std::vector<ListComponentData>>(props, listItemsSymbol);
    modassert(listItemPtr, "invalid listItems prop");
 
    auto listItems = *listItemPtr;

    std::vector<Component> elements;

    auto titleValue = strFromProp(props, titleSymbol, "title placeholder");
    auto titleTextbox = withPropsCopy(textbox, Props {
      .props = {
        PropPair { .symbol = valueSymbol, .value = titleValue },
      }
    });
    elements.push_back(titleTextbox);

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
    //////////////////////////////

    auto listBoundingBox = simpleVerticalLayout(elements).draw(drawTools, props);
    //drawDebugBoundingBox(drawTools, listBoundingBox, glm::vec4(0.f, 1.f, 0.f, 1.f));
    auto boundingBox = listBoundingBox;
    auto sides = calculateSides(boundingBox);

    auto onClickX = fnFromProp(props, onclickSymbol);
    if (onClickX.has_value()){
      auto xMappingId = uniqueMenuItemMappingId();
      bool xHovered =  drawTools.selectedId.has_value() && drawTools.selectedId.value() == xMappingId;
      drawTextLeftHorzDownVert(drawTools, "x", sides.right, sides.top, 0.04f, xHovered ? glm::vec4(1.f, 1.f, 1.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 0.4f), xMappingId);
      drawTools.registerCallbackFns(xMappingId, onClickX.value());
    }

    //drawTools.drawText("some value", sides.right, sides.top, fontSize, false, std::nullopt, std::nullopt, true, std::nullopt, uniqueMenuItemMappingId());
    return boundingBox;
  },
};
