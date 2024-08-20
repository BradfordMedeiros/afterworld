#include "./colorpicker.h"


Component createRgbSlider(std::string label, float percentage, std::function<void(float)> onSlide){
  Slider sliderData {
    .min = 0.f,
    .max = 10.f,
    .percentage = percentage,
    .onSlide = onSlide,
  };
  Props sliderProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = label },
      PropPair { .symbol = sliderSymbol, .value = sliderData },
    },
  };
  auto sliderWithProps = withPropsCopy(slider, sliderProps);
  return sliderWithProps;
}

Component colorDisplay {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
  	auto color = vec4FromProp(props, colorSymbol, glm::vec4(0.f, 0.f, 0.f, 0.f));
    drawTools.drawRect(0.05f, 0.05f, 0.2f, 0.2f, false, color, std::nullopt, true, std::nullopt /* selection id */, std::nullopt  /* texture */, std::nullopt, std::nullopt);
    BoundingBox2D boundingBox {
    	.x = 0.f,
    	.y = 0.f,
    	.width = 0.3f,
    	.height = 0.3f,
    };
    return boundingBox;
  },
};

Component colorPickerComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto activeColor = vec4FromProp(props, tintSymbol, glm::vec4(0.f, 0.f, 0.f, 0.f));
    auto onSlidePtr = typeFromProps<std::function<void(glm::vec4)>>(props, onSlideSymbol);
    modassert(onSlidePtr, "slide must not be null");
    auto onSlide = *onSlidePtr;

    std::vector<Component> elements;
    elements.push_back(createRgbSlider("red", activeColor.r, [activeColor, onSlide](float r) -> void { 
      glm::vec4 newColor = activeColor;
      newColor.r = r;
      onSlide(newColor);
    }));
    elements.push_back(createRgbSlider("green", activeColor.g, [activeColor, onSlide](float g) -> void { 
      glm::vec4 newColor = activeColor;
      newColor.g = g;
      onSlide(newColor);
    }));
    elements.push_back(createRgbSlider("blue", activeColor.b, [activeColor, onSlide](float b) -> void { 
      glm::vec4 newColor = activeColor;
      newColor.b = b;
      onSlide(newColor);
    }));
    elements.push_back(createRgbSlider("alpha", activeColor.a, [activeColor, onSlide](float a) -> void { 
      glm::vec4 newColor = activeColor;
      newColor.a = a;
      onSlide(newColor);
    }));
    elements.push_back(withPropsCopy(colorDisplay, Props {
    	.props = { 
    		{ .symbol = colorSymbol, .value = activeColor },
    	}
    }));

    auto colorPickInner = simpleVerticalLayout(elements, glm::vec2(0.f, 0.f), defaultAlignment, styles.mainBorderColor, 0.f, styles.primaryColor);
    return  colorPickInner.draw(drawTools, props);
  },
};


