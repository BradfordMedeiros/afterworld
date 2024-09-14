#include "./elevator.h"

Props noClickButtonProps { 
  .props = { 
    PropPair { .symbol = valueSymbol, .value = std::string("DISABLED") },
    PropPair { .symbol = paddingSymbol, .value = 0.4f },
  }
};
Component noClickButton = withPropsCopy(button, noClickButtonProps);
  
Component elevatorComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), true, std::nullopt, "./res/textures/test.png", std::nullopt, std::nullopt);
    ElevatorUiOptions* elevatorOptions = typeFromProps<ElevatorUiOptions>(props, valueSymbol);
    modassert(elevatorOptions, "elevator options not passed in");

    std::vector<Component> children = {};

    if (elevatorOptions -> canClickUp){
      Props upButtonProps { 
        .props = { 
          PropPair { .symbol = valueSymbol, .value = std::string("UP") },
          PropPair { .symbol = onclickSymbol, .value = elevatorOptions -> onClickUp },
          PropPair { .symbol = paddingSymbol, .value = 0.4f },
        }
      };
      auto upButton = withPropsCopy(button, upButtonProps);
      children.push_back(upButton);
    }else{
      children.push_back(noClickButton);
    }

    if (elevatorOptions -> canClickDown){
      Props downButtonProps { 
        .props = { 
          PropPair { .symbol = valueSymbol, .value = std::string("DOWN") },
          PropPair { .symbol = onclickSymbol, .value = elevatorOptions -> onClickDown },
          PropPair { .symbol = paddingSymbol, .value = 0.4f },
        }
      };
      auto downButton = withPropsCopy(button, downButtonProps);
      children.push_back(downButton);
    }else{
      children.push_back(noClickButton);
    }


    Layout layout {
      .tint = glm::vec4(0.f, 0.f, 1.f, 0.5f),
      .showBackpanel = true,
      .borderColor = std::nullopt,
      .minwidth = 0.f,
      .minheight = 0.f,
      .layoutType = LAYOUT_VERTICAL2,
      .layoutFlowHorizontal = UILayoutFlowNone2,
      .layoutFlowVertical = UILayoutFlowNone2,
      .alignHorizontal = UILayoutFlowNone2,
      .alignVertical = UILayoutFlowNone2,
      .spacing = 0.f,
      .minspacing = 0.f,
      .padding = 0.f,
      .children = children,
    };
    Props listLayoutProps {
      .props = {
        { .symbol = yoffsetSymbol, .value = 0.f },
        { .symbol = layoutSymbol, .value = layout },
      },
    };
    return layoutComponent.draw(drawTools, listLayoutProps);;
  },
};
