#include "./console.h"

const int CONSOLE_WIDTH = 2.f;
const int CONSOLE_HEIGHT = 2.f;
const float ELEMENT_PADDING = 0.02f;
const float ELEMENT_WIDTH = 1.f;

extern bool debugLayout;

std::vector<std::string> commandHistory = {
  "poke some value", 
  "another test command",
  "wow",
  "poke some value", 
  "another test command",
  "wow",
  "poke some value", 
  "another test command",
  "wow",
  "poke some value", 
  "another test command",
  "wow",
};
int selectedIndex = 0;


Component createTitle(std::string&& item){
  Props listItemProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = item },
      PropPair { .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 1.f, 0.4f) },
      PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.8f) },
      PropPair { .symbol = minwidthSymbol, .value = ELEMENT_WIDTH },
      PropPair { .symbol = paddingSymbol, .value = ELEMENT_PADDING },

      //PropPair { .symbol = onclickSymbol, .value = onClick },
    },
  };
  auto listItemWithProps = withPropsCopy(listItem, listItemProps); 
  return listItemWithProps;
}

Component createConsoleItem(std::string& item,  int index){
  std::function<void()> onClick = [index]() -> void {
    std::cout << "on click" << std::endl;
    selectedIndex = index;
  };
  Props listItemProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = item },
      //PropPair { .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 0.f, 0.f) },
      PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.8f) },
      PropPair { .symbol = minwidthSymbol, .value = ELEMENT_WIDTH },
      PropPair { .symbol = paddingSymbol, .value = ELEMENT_PADDING },
      PropPair { .symbol = onclickSymbol, .value = onClick },
    },
  };
  if (selectedIndex == index){
    listItemProps.props.push_back(PropPair { .symbol = tintSymbol, .value = glm::vec4(0.4f, 0.4f, 0.4f, 0.8f) });
  }
  auto listItemWithProps = withPropsCopy(listItem, listItemProps); 
  return listItemWithProps;
}


TextData textboxConfigData {
  .valueText = "somedebugtext",
  .cursorLocation = 0,
  .highlightLength = 0,
  .maxchars = -1,
};
Component consoleComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
  	std::vector<Component> elements = {};

    elements.push_back(createTitle("History"));

    for (int i = 0; i < commandHistory.size(); i++){
      elements.push_back(createConsoleItem(commandHistory.at(i), i));
    }


    std::function<void(TextData)> onEdit = [](TextData textData) -> void {
      std::cout << "on edit" << std::endl;
      assert(false);
      textboxConfigData = textData;
      //textboxOptions.onEdit(textData.valueText);
    };
    TextData textData {
      .valueText = textboxConfigData.valueText,
      .cursorLocation = textboxConfigData.cursorLocation,
      .highlightLength = textboxConfigData.highlightLength,
      .maxchars = textboxConfigData.maxchars,
    };
    Props textboxProps {
      .props = {
        PropPair { .symbol = editableSymbol, .value = true },
        PropPair { .symbol = textDataSymbol, .value = textData },
        PropPair { .symbol = onInputSymbol, .value = onEdit },
        //PropPair { .symbol = valueSymbol, .value = textboxConfigData.valueText },
      }
    };
    auto textboxWithProps = withPropsCopy(textbox, textboxProps);
    elements.push_back(textboxWithProps);

  	Layout layout {
  	  .tint = glm::vec4(0.f, 0.f, 0.f, 0.9f),
  	  .showBackpanel = true,
  	  .borderColor = glm::vec4(1.f, 1.f, 1.f, 1.f),
  	  .minwidth = CONSOLE_WIDTH,
  	  .minheight = CONSOLE_HEIGHT,
  	  .layoutType = LAYOUT_VERTICAL2,
  	  .layoutFlowHorizontal = UILayoutFlowNone2,
  	  .layoutFlowVertical = UILayoutFlowNone2,
  	  .alignHorizontal = UILayoutFlowNegative2,
  	  .alignVertical = UILayoutFlowPositive2,
  	  .spacing = 0.f,
  	  .minspacing = 0.f,
  	  .padding = styles.dockElementPadding,
  	  .children = elements,
  	};
  	Props listLayoutProps {
  	  .props = {
  	    { .symbol = layoutSymbol, .value = layout },
  	  },
  	};
  	auto layoutScenegraph = withPropsCopy(layoutComponent, listLayoutProps);
  	auto boundingBox = layoutScenegraph.draw(drawTools, props);
    return boundingBox;
  },
};