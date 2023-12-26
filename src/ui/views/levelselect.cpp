#include "./levelselect.h"

int selectedLevel = 0;
struct UILevel {
  std::string name;
};
std::vector<UILevel> levels = {
  UILevel { .name = "Dreams and Reconciliation" },
  UILevel { .name = "Arrival" },
  UILevel { .name = "Ocean Diving and Submarine Voyages" },
  UILevel { .name = "Departure" },
  UILevel { .name = "Hybrid Nightmare" },
  UILevel { .name = "Nightmare" },
  UILevel { .name = "Dreams and Reconciliation II" },
  UILevel { .name = "Odyssey" },
};

std::function<void(int)> onSelectLevel = [](int levelIndex) -> void {
  std::cout << "on select level: " << levelIndex << std::endl;
  selectedLevel = levelIndex;
};

const float selectorRatio = (1.f / 3.f);

Component levelMenu {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    std::function<void(int)>* onClickPtr = typeFromProps<std::function<void(int)>>(props, onclickSymbol);
    std::function<void(int)> onClick = *onClickPtr;

    std::vector<Component> levelChoices;

    {
      Props listItemProps {
        .props = {
          PropPair { .symbol = valueSymbol, .value = std::string("LEVELS") },
          PropPair { .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 0.f, 0.2f) },
          PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.8f) },
          PropPair { .symbol = paddingSymbol, .value = styles.dockElementPadding },
          //PropPair { .symbol = onclickSymbol, .value = onClick },
          PropPair { .symbol = minwidthSymbol, .value = 0.5f },
          PropPair { .symbol = borderColorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 0.1f) },
          PropPair { .symbol = fontsizeSymbol, .value = 0.03f }
        },
      };
  
      auto listItemWithProps = withPropsCopy(listItem, listItemProps);
      levelChoices.push_back(listItemWithProps);
    }

    for (int i = 0; i < levels.size(); i++){
      std::function<void()> onClickLevel = [i, onClick]() -> void {
        onClick(i);
      };
      Props listItemProps {
        .props = {
          PropPair { .symbol = valueSymbol, .value = levels.at(i).name },
          PropPair { .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 0.f, 0.2f) },
          PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 0.8f) },
          PropPair { .symbol = paddingSymbol, .value = styles.dockElementPadding },
          PropPair { .symbol = onclickSymbol, .value = onClickLevel },
          PropPair { .symbol = minwidthSymbol, .value = 0.5f },
          PropPair { .symbol = borderColorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 0.1f) },
          PropPair { .symbol = fontsizeSymbol, .value = 0.02f }
        },
      };

      auto listItemWithProps = withPropsCopy(listItem, listItemProps);
      levelChoices.push_back(listItemWithProps);
    }


    Layout selectorLayout {
      .tint = glm::vec4(1.f, 1.f, 1.f, 0.f),
      .showBackpanel = true,
      .borderColor = glm::vec4(0.f, 0.f, 0.f, 0.f), //styles.highlightColor,
      .minwidth = selectorRatio * 2.f,
      .minheight = 2.f,
      .layoutType = LAYOUT_VERTICAL2, // LAYOUT_VERTICAL2,
      .layoutFlowHorizontal = UILayoutFlowNone2, // L UILayoutFlowNone2,
      .layoutFlowVertical = UILayoutFlowNone2,
      .alignHorizontal = UILayoutFlowNone2,
      .alignVertical = UILayoutFlowPositive2,
      .spacing = 0.f,
      .minspacing = 0.f,
      .padding = 0.2f,
      .children = levelChoices,
    };

    Props selectorLayoutProps {
      .props = {
        { .symbol = layoutSymbol, .value = selectorLayout },
      },
    };

    auto selectLayoutComponent = withProps(layoutComponent, selectorLayoutProps);
    return selectLayoutComponent.draw(drawTools, props);
  },
};

Component levelDetail {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    std::string* levelName = typeFromProps<std::string>(props, valueSymbol);

    std::vector<Component> detailElements;

    {
      Props listItemProps {
          .props = {
            PropPair { .symbol = tintSymbol, .value = glm::vec4(0.3f, 0.3f, 0.3f, 0.4f) },
            PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 0.4f) },
            PropPair { .symbol = paddingSymbol, .value = 0.2f },
            PropPair { .symbol = minwidthSymbol, .value = 1.f },
            //PropPair { .symbol = onclickSymbol, .value = onClick },
          },
        };
        if (levelName){
          listItemProps.props.push_back(PropPair {
            .symbol = valueSymbol,
            .value = *levelName,
          });
        }else{
          listItemProps.props.push_back(PropPair {
            .symbol = valueSymbol,
            .value = std::string("no level selected"),
          });
        }
  
      auto listItemWithProps = withPropsCopy(listItem, listItemProps);
      detailElements.push_back(listItemWithProps);
    }



    Layout levelDisplayLayout {
      .tint = glm::vec4(0.f, 0.f, 0.f, 0.2f),
      .showBackpanel = true,
      .borderColor = glm::vec4(0.f, 0.f, 0.f, 0.f), //styles.highlightColor,
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
      .children = detailElements,
    };

    Props levelDisplayLayoutProps {
      .props = {
        { .symbol = layoutSymbol, .value = levelDisplayLayout },
      },
    };

    auto levelDisplayLayoutComponent = withProps(layoutComponent, levelDisplayLayoutProps);
    return levelDisplayLayoutComponent.draw(drawTools, props);
  },
};

Component levelSelectComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    std::vector<Component> elements;

   
    Props levelSelectorProps {
      .props = {
        PropPair { .symbol = onclickSymbol, .value = onSelectLevel },
      }
    };
    elements.push_back(withProps(levelMenu, levelSelectorProps));


    Props levelDetailProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = levels.at(selectedLevel).name },
      }
    };
    elements.push_back(withProps(levelDetail, levelDetailProps));


    /////////////////////////////////////////



    Layout outerLayout {
      .tint = glm::vec4(1.f, 1.f, 1.f, 0.f),
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
