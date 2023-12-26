#include "./levelselect.h"

Component levelSelectComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    std::vector<Component> elements;


    std::vector<Component> placeholderElements;
    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = std::string("placeholder") },
        PropPair { .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 0.f, 1.f) },
        PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.8f) },
        PropPair { .symbol = paddingSymbol, .value = 0.01f },
        //PropPair { .symbol = onclickSymbol, .value = onClick },
      },
    };

    auto listItemWithProps = withPropsCopy(listItem, listItemProps);
    placeholderElements.push_back(listItemWithProps);

    /////////////////////////

    float selectorRatio = (1.f / 3.f);

    Layout selectorLayout {
      .tint = glm::vec4(0.f, 0.f, 1.f, 0.8f),
      .showBackpanel = true,
      .borderColor = styles.highlightColor,
      .minwidth = selectorRatio * 2.f,
      .minheight = 2.f,
      .layoutType = LAYOUT_VERTICAL2, // LAYOUT_VERTICAL2,
      .layoutFlowHorizontal = UILayoutFlowNone2, // L UILayoutFlowNone2,
      .layoutFlowVertical = UILayoutFlowNone2,
      .alignHorizontal = UILayoutFlowNegative2,
      .alignVertical = UILayoutFlowNone2,
      .spacing = 0.f,
      .minspacing = 0.f,
      .padding = 0.f,
      .children = placeholderElements,
    };

    Props selectorLayoutProps {
      .props = {
        { .symbol = layoutSymbol, .value = selectorLayout },
      },
    };

    auto selectLayoutComponent = withProps(layoutComponent, selectorLayoutProps);
    elements.push_back(selectLayoutComponent);

    /////////////////////////////////////////


    /////////////////////////


    Layout levelDisplayLayout {
      .tint = glm::vec4(1.f, 0.f, 1.f, 0.8f),
      .showBackpanel = true,
      .borderColor = styles.highlightColor,
      .minwidth = (1.f - selectorRatio) * 2.f,
      .minheight = 2.f,
      .layoutType = LAYOUT_VERTICAL2, // LAYOUT_VERTICAL2,
      .layoutFlowHorizontal = UILayoutFlowNone2, // L UILayoutFlowNone2,
      .layoutFlowVertical = UILayoutFlowNone2,
      .alignHorizontal = UILayoutFlowNegative2,
      .alignVertical = UILayoutFlowNone2,
      .spacing = 0.f,
      .minspacing = 0.f,
      .padding = 0.f,
      .children = placeholderElements,
    };

    Props levelDisplayLayoutProps {
      .props = {
        { .symbol = layoutSymbol, .value = levelDisplayLayout },
      },
    };

    auto levelDisplayLayoutComponent = withProps(layoutComponent, levelDisplayLayoutProps);
    elements.push_back(levelDisplayLayoutComponent);

    /////////////////////////////////////////



    Layout outerLayout {
      .tint = glm::vec4(1.f, 1.f, 1.f, 0.8f),
      .showBackpanel = true,
      .borderColor = styles.highlightColor,
      .minwidth = 2.f,
      .minheight = 2.f,
      .layoutType = LAYOUT_HORIZONTAL2, 
      .layoutFlowHorizontal = UILayoutFlowNone2,
      .layoutFlowVertical = UILayoutFlowNone2,
      .alignHorizontal = UILayoutFlowNegative2,
      .alignVertical = UILayoutFlowNone2,
      .spacing = 0.f,
      .minspacing = 0.f,
      .padding = 0.f,
      .children = elements,
    };

    Props listLayoutProps {
      .props = {
        { .symbol = layoutSymbol, .value = outerLayout },
      },
    };

    auto boundingBox = withProps(layoutComponent, listLayoutProps).draw(drawTools, props);
    //drawDebugBoundingBox(drawTools, cameraBoundingBox, glm::vec4(1.f, 0.f, 0.f, 1.f));
    return boundingBox;
  },
};
