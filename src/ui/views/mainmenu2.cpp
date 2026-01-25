#include "./mainmenu.h"


bool onMenu2NewGameClick();

Props createLevelListPropsBall(float offset){
  std::vector<ListComponentData> levels;
  levels.push_back(ListComponentData {
    .name = "New Game",
    .onClick = []() -> void {
      //pushHistory({ "levelselect" }, false);
      onMenu2NewGameClick();
    }
  });
  levels.push_back(ListComponentData {
    .name = "Continue",
    .onClick = []() -> void {
      //pushHistory({ "settings" }, false);
    }
  });
  levels.push_back(ListComponentData {
    .name = "Quit",
    .onClick = []() -> void {
      modlog("exit", "exit normally through main menu");
      exit(0);
    }
  });


  Props levelProps {
    .props = {
      PropPair { .symbol = listItemsSymbol, .value = levels },
      PropPair { .symbol = xoffsetSymbol,   .value = 0.f  },
      PropPair { .symbol = yoffsetSymbol,   .value = 0.2f - offset  },
      PropPair { .symbol = colorSymbol,      .value = glm::vec4(0.f, 0.f, 0.f, 0.3f) },
      PropPair { .symbol = flowVertical,    .value = UILayoutFlowNegative2 },
      PropPair { .symbol = itemPaddingSymbol, .value = 0.04f },
      PropPair { .symbol = fontsizeSymbol, .value = 0.02f },
    },
  };
  return levelProps;
}

Component mainMenu2 {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    static std::string title("Soul Delivery");

    auto mainMenuOptions = typeFromProps<MainMenu2Options>(props, valueSymbol);
    modassert(mainMenuOptions, "no main menu2 options");

    float alpha = mainMenuOptions -> backgroundColor.w;

    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = title },
        PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 0.8f) },
        PropPair { .symbol = fontsizeSymbol, .value = 0.1f }
      },
    };

    auto listItemWithProps = withPropsCopy(listItem, listItemProps);

    std::vector<Component> children = { listItemWithProps };
    Layout layout {
      .tint = glm::vec4(0.f, 0.f, 0.f, 0.5f),
      .showBackpanel = false,
      .borderColor = glm::vec4(1.f, 0.f, 0.f, 0.f),
      .minwidth = 0.f,
      .minheight = 0.f,
      .layoutType = LAYOUT_HORIZONTAL2,
      .layoutFlowHorizontal = UILayoutFlowNone2,
      .layoutFlowVertical = UILayoutFlowNone2,
      .alignHorizontal = UILayoutFlowNone2,
      .alignVertical = UILayoutFlowNone2,
      .spacing = 0.01f,
      .minspacing = 0.f,
      .padding = 0.f,
      .children = children,
    };
    Props listLayoutProps {
      .props = {
        { .symbol = yoffsetSymbol, .value = 0.4f + mainMenuOptions -> offsetY },
        { .symbol = xoffsetSymbol, .value = 0.f },
        { .symbol = layoutSymbol, .value = layout },
      },
    };
    layoutComponent.draw(drawTools, listLayoutProps);

    bool useCoolShader = false;
    static bool didLoadShader = false;
    static unsigned int* shaderId = 0;
    if (useCoolShader){
      if (!didLoadShader){
        shaderId = gameapi -> loadShader("storyboard", "../afterworld/shaders/storyboard");
        didLoadShader = true;
      }
    }

    drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, mainMenuOptions -> backgroundColor, true, std::nullopt, "./res/textures/wood.jpg",  ShapeOptions {
      .shaderId = useCoolShader ? std::optional<unsigned int>(*shaderId) : std::optional<unsigned int>(std::nullopt),
      .zIndex = -1,
    }, std::nullopt);

    auto levelSelection = withPropsCopy(listComponent, createLevelListPropsBall(mainMenuOptions -> offsetY));
    return levelSelection.draw(drawTools, props);
  },
};