#include "./dock_slider.h"

Component createDockSlider(DockSliderConfig& dockSlider){
  Slider sliderData {
    .min = dockSlider.min,
    .max = dockSlider.max,
    .percentage = dockSlider.percentage(),
    .onSlide = dockSlider.onSlide,
  };
  Props sliderProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = dockSlider.label },
      PropPair { .symbol = sliderSymbol, .value = sliderData },
    },
  };
  auto sliderWithProps = withPropsCopy(slider, sliderProps); 
  return sliderWithProps;
}