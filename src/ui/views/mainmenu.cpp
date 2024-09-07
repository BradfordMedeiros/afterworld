#include "./mainmenu.h"

void pushHistory(std::vector<std::string> route, bool replace);

Props createLevelListProps(){
  std::vector<ListComponentData> levels;
  levels.push_back(ListComponentData {
    .name = "Campaign",
    .onClick = []() -> void {
      pushHistory({ "levelselect" }, false);
    }
  });
  levels.push_back(ListComponentData {
    .name = "Settings",
    .onClick = []() -> void {
      pushHistory({ "settings" }, false);
    }
  });
  levels.push_back(ListComponentData {
    .name = "Model Viewer",
    .onClick = []() -> void {
      pushHistory({ "modelviewer" }, false);
    }
  });
  levels.push_back(ListComponentData {
    .name = "Particle Viewer",
    .onClick = []() -> void {
      pushHistory({ "particleviewer" }, false);
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
      //PropPair { .symbol = xoffsetSymbol,   .value = 0.f },
      PropPair { .symbol = yoffsetSymbol,   .value = 0.2f },
      //PropPair { .symbol = tintSymbol,      .value = glm::vec4(0.f, 0.f, 0.f, 0.3f) },
      PropPair { .symbol = flowVertical,    .value = UILayoutFlowNegative2 },
      PropPair { .symbol = itemPaddingSymbol, .value = 0.04f },
    },
  };
  return levelProps;
}

Component mainMenu {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = std::string("AFTERWORLD") },
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
        { .symbol = yoffsetSymbol, .value = 0.4f },
        { .symbol = layoutSymbol, .value = layout },
      },
    };
    layoutComponent.draw(drawTools, listLayoutProps);

    auto levelSelection = withPropsCopy(listComponent, createLevelListProps());
    return levelSelection.draw(drawTools, props);
  },
};