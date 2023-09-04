#include "./colorpicker.h"


Component createRgbSlider(float percentage){
  Slider sliderData {
    .min = 0.f,
    .max = 10.f,
    .percentage = percentage,
    .update = false,
  };
  Props sliderProps {
    .props = {
      PropPair { .symbol = sliderSymbol, .value = sliderData },
    },
  };
  auto sliderWithProps = withPropsCopy(slider, sliderProps);
  return sliderWithProps;
}

Component colorDisplay {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
  	auto color = vec4FromProp(props, colorSymbol, glm::vec4(0.f, 0.f, 0.f, 0.f));
    drawTools.drawRect(0.f, 0.f, 0.2f, 0.2f, false, color, std::nullopt, true, std::nullopt /* selection id */, std::nullopt  /* texture */);
    BoundingBox2D boundingBox {
    	.x = 0.f,
    	.y = 0.f,
    	.width = 0.2f,
    	.height = 0.2f,
    };
    return boundingBox;
  },
};

Component colorPickerComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    static glm::vec4* activeColor = static_cast<glm::vec4*>(uiConnect(color));

    std::vector<Component> elements;
    elements.push_back(createRgbSlider(activeColor -> r));
    elements.push_back(createRgbSlider(activeColor -> g));
    elements.push_back(createRgbSlider(activeColor -> b));
    elements.push_back(createRgbSlider(activeColor -> a));
    elements.push_back(withPropsCopy(colorDisplay, Props {
    	.props = { 
    		{ .symbol = colorSymbol, .value = *activeColor },
    	}
    }));

    /////////////////////////
    auto titleValue = strFromProp(props, titleSymbol, "Color Selection");
    auto uiWindowComponent = createUiWindow(elements, titleValue);

  std::function<void()> onWindowDrag = []() -> void {
    std::cout << "on window drag" << std::endl;
    exit(1);
  };
    auto boundingBox = uiWindowComponent.draw(drawTools, props);
    auto onClickX = fnFromProp(props, onclickSymbol);
    if (true ){
      drawWindowX(drawTools, boundingBox, onWindowDrag);
    }
    return boundingBox;
  },
};