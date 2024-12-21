#include "./levelselect.h"

extern CustomApiBindings* gameapi;

int selectedLevel = 0;
struct UILevel {
  std::string name;
  std::string description;
  std::string image;
  std::string shortcut;
};

std::vector<UILevel> queryLevels(){
  auto query = gameapi -> compileSqlQuery("select name, description, image, shortcut from levels", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query queryLevels");

  std::vector<UILevel> levels;
  for (auto &row : result){
    levels.push_back(UILevel{
      .name = row.at(0),
      .description = row.at(1),
      .image = row.at(2),
      .shortcut = row.at(3),
    });
  }
  return levels;
}

std::function<void(int)> onSelectLevel = [](int levelIndex) -> void {
  std::cout << "on select level: " << levelIndex << std::endl;
  selectedLevel = levelIndex;
};

const float selectorRatio = (1.f / 3.f);

Component levelMenu {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    std::vector<UILevel>** levelPtr = typeFromProps<std::vector<UILevel>*>(props, valueSymbol);
    std::vector<UILevel>& levels = **levelPtr;
    std::function<void(int)>* onClickPtr = typeFromProps<std::function<void(int)>>(props, onclickSymbol);
    std::function<void(int)> onClick = *onClickPtr;
    
    std::function<void(std::optional<std::string>)>* playLevelPtr = typeFromProps<std::function<void(std::optional<std::string>)>>(props, playLevelSymbol);
    modassert(playLevelPtr, "levelMenu playLevel not provided");
    auto playLevel = *playLevelPtr;

    std::vector<Component> levelChoices;

    // level header
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

    // level choices 
    for (int i = 0; i < levels.size(); i++){
      std::function<void()> onClickLevel = [i, onClick]() -> void {
        onClick(i);
      };
      Props listItemProps {
        .props = {
          PropPair { .symbol = valueSymbol, .value = levels.at(i).name },
          PropPair { .symbol = tintSymbol, .value = ((i == selectedLevel) ? glm::vec4(1.f, 1.f, 0.f, 0.2f) : glm::vec4(0.f, 0.f, 0.f, 0.2f)) },
          PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 0.8f) },
          PropPair { .symbol = paddingSymbol, .value = styles.dockElementPadding },
          PropPair { .symbol = onclickSymbol, .value = onClickLevel },
          PropPair { .symbol = minwidthSymbol, .value = 0.5f },
          //PropPair { .symbol = borderColorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 0.1f) },
          PropPair { .symbol = fontsizeSymbol, .value = 0.02f }
        },
      };

      auto listItemWithProps = withPropsCopy(listItem, listItemProps);
      levelChoices.push_back(listItemWithProps);
    }

    // play button
    {

      std::function<void()> onClickLevel = [&levels, selectedLevel, playLevel]() -> void {
        modlog("ui play", std::to_string(selectedLevel) + " - " + levels.at(selectedLevel).shortcut);
        playLevel(levels.at(selectedLevel).shortcut);
      }; 
      Props listItemProps {
        .props = {
          PropPair { .symbol = valueSymbol, .value = std::string("START") },
          PropPair { .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 0.f, 1.f) },
          PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 0.8f) },
          PropPair { .symbol = paddingSymbol, .value = styles.dockElementPadding },
          PropPair { .symbol = onclickSymbol, .value = onClickLevel },
          PropPair { .symbol = minwidthSymbol, .value = 0.5f },
          //PropPair { .symbol = borderColorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 0.8f) },
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

Component createDetailText(std::string&& detailText){
  Props listItemProps {
    .props = {
      //PropPair { .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 0.5f, 0.5f) },
      PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 0.4f) },
      PropPair { .symbol = paddingSymbol, .value = styles.dockElementPadding },
      PropPair { .symbol = minwidthSymbol, .value = 1.f },
      PropPair { .symbol = valueSymbol, .value = detailText },
    },
  };
  auto listItemWithProps = withPropsCopy(listItem, listItemProps);
  return listItemWithProps;
}

Component levelDetail {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    UILevel* level = typeFromProps<UILevel>(props, valueSymbol);
    modassert(level, "level must be supplied to level detail");

    std::vector<Component> detailElements;
    
    float width = (1.f - selectorRatio) * 2.f;
    Props imageProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = level -> image },
        PropPair { .symbol = widthSymbol, .value = width },
      }
    };
    detailElements.push_back(withPropsCopy(imageComponent, imageProps));
    

    {
      Props listItemProps {
        .props = {
          //PropPair { .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 0.5f, 0.5f) },
          PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 0.4f) },
          PropPair { .symbol = paddingSymbol, .value = styles.dockElementPadding },
          PropPair { .symbol = minwidthSymbol, .value = 1.f },
          PropPair { .symbol = valueSymbol, .value = level -> description },
        },
      };
      auto listItemWithProps = withPropsCopy(listItem, listItemProps);
      detailElements.push_back(listItemWithProps);
    }

    
    detailElements.push_back(createDetailText("Highest Difficulty Completed:  None"));
    detailElements.push_back(createDetailText("High Score:  N/A"));

    Layout levelDisplayLayout {
      .tint = glm::vec4(0.f, 0.f, 0.f, 0.4f),
      .showBackpanel = true,
      .borderColor = glm::vec4(0.f, 0.f, 0.f, 1.f), //styles.highlightColor,
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
    static std::vector<UILevel> levels = queryLevels();
    std::vector<Component> elements;

    std::function<void(std::optional<std::string>)>* playLevelPtr = typeFromProps<std::function<void(std::optional<std::string>)>>(props, playLevelSymbol);
    modassert(playLevelPtr, "levelSelectComponent playLevel not provided");
    
    Props levelSelectorProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = &levels },
        PropPair { .symbol = onclickSymbol, .value = onSelectLevel },
        PropPair { .symbol = playLevelSymbol, .value = *playLevelPtr },
      }
    };
    elements.push_back(withProps(levelMenu, levelSelectorProps));


    Props levelDetailProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = levels.at(selectedLevel) },
      }
    };
    elements.push_back(withProps(levelDetail, levelDetailProps));

    Layout outerLayout {
      .tint = glm::vec4(0.f, 0.f, 0.f, 0.f),
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
