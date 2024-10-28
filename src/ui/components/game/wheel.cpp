#include "./wheel.h"

extern CustomApiBindings* gameapi;

Component wheelComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto wheelConfigPtr = typeFromProps<WheelConfig>(props, valueSymbol);
    modassert(wheelConfigPtr, "wheelComponent wheelConfigPtr is null");
    WheelConfig& wheelConfig = *wheelConfigPtr;

  	for (int i = 0; i < wheelConfig.numElementsInWheel; i++){
  		if ((i + 1) > wheelConfig.numElementsToShow){
  			break;
			}
  		float value = wheelConfig.getRotationOffset() + i * (2 * M_PI) / wheelConfig.numElementsInWheel;
  		float midpointX = glm::cos(value) * wheelConfig.wheelRadius;
  		float midpointY = glm::sin(value) * wheelConfig.wheelRadius;

  		auto index = i + wheelConfig.offset;
	    auto textContent = wheelConfig.wheelContents.at(index % wheelConfig.wheelContents.size());
  	
  		std::function<void()> onClick = [index, &wheelConfig]() -> void {
  			wheelConfig.onClick(index);
  		};

			Props listItemProps {
				.props = {
					PropPair { .symbol = valueSymbol, .value = textContent },
					PropPair { .symbol = paddingSymbol, .value = 0.1f },
					PropPair { .symbol = onclickSymbol, .value = onClick },
					PropPair { .symbol = tintSymbol, .value =  glm::vec4(0.f, 0.f, 0.f, 0.2f) },
				},
			};
			if (index == wheelConfig.selectedIndex){
				listItemProps.props.push_back(PropPair { .symbol = borderColorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 1.f) });
				listItemProps.props.push_back(PropPair { .symbol = focusTintSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 1.f) });
			}else{
				listItemProps.props.push_back(PropPair { .symbol = borderColorSymbol, .value = glm::vec4(1.f, 0.f, 1.f, 1.f) });
			}
      std::vector<Component> wheelButton = { withPropsCopy(listItem, listItemProps) };

		 	auto layout = simpleHorizontalLayout(wheelButton, 0.f, glm::vec4(0.f, 0.f, 0.f, 0.f));
      Props layoutProps { 
        .props = { 
          PropPair { .symbol = xoffsetSymbol, .value = midpointX },
          PropPair { .symbol = yoffsetSymbol, .value = midpointY },
        }
      };
		 	layout.draw(drawTools, layoutProps);
  	}

  	// put this whole thing in a layout so can have a background and dimensions on this

  	return BoundingBox2D {
  		.x = 0.f,
  		.y = 0.f,
  		.width = 1.f,
  		.height = 1.f,
  	};
  },
};
