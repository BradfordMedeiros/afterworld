#include "./mainmenu.h"

void pushHistory(std::vector<std::string> route, bool replace);
std::string getArgOption(const char* name);
bool hasOption(const char* name);

extern CustomApiBindings* gameapi;

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
      PropPair { .symbol = colorSymbol,      .value = glm::vec4(0.f, 0.f, 0.f, 0.3f) },
      PropPair { .symbol = flowVertical,    .value = UILayoutFlowNegative2 },
      PropPair { .symbol = itemPaddingSymbol, .value = 0.04f },
      PropPair { .symbol = fontsizeSymbol, .value = 0.02f },
    },
  };
  return levelProps;
}

Component mainMenu {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {

    /*{
      // Drawing a 2d layer background for the menu...i prefer this
      //static unsigned int* shaderId = gameapi -> loadShader("menu", "../afterworld/shaders/menu");  // 2d shader only remember
      float length = 60.f;
      float remainder = std::fmod(gameapi -> timeSeconds(true), length);
      float percentage = remainder / length;
      std::cout << "mainmenu remainder: " << percentage << std::endl;
  
      float width = 4.f;
      const char* filepath = "../gameresources/skybox/space/front.jpg";

      float intensity = 0.5f * (glm::cos(gameapi -> timeSeconds(true)) + 1);
      drawTools.drawRect(0.f + (percentage * width), 0.f, width, width, false, glm::vec4(intensity, intensity, intensity, 1.f), true, std::nullopt, filepath, std::nullopt, std::nullopt);
      drawTools.drawRect(-1 * width + (percentage * width), 0.f, width, width, false, glm::vec4(intensity, intensity, intensity, 1.f), true, std::nullopt, filepath,  std::nullopt, std::nullopt);

      drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(0.6f, 0.6f, 0.6f, 1.f), true, std::nullopt,  "./res/textures/wood.png",  std::nullopt, std::nullopt);
    }*/

    static std::string title("AFTERWORLD");
    static bool getTitleOnce = false;
    if (!getTitleOnce){
      getTitleOnce = true;
      if (hasOption("title")){
        title = getArgOption("title");
      }
    }
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
        { .symbol = yoffsetSymbol, .value = 0.4f },
        { .symbol = layoutSymbol, .value = layout },
      },
    };
    layoutComponent.draw(drawTools, listLayoutProps);


    auto levelSelection = withPropsCopy(listComponent, createLevelListProps());
    return levelSelection.draw(drawTools, props);
  },
};