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
      PropPair { .symbol = paddingSymbol, .value = styles.dockElementPadding },
      PropPair { .symbol = valueSymbol, .value = dockSlider.label },
      PropPair { .symbol = sliderSymbol, .value = sliderData },
      PropPair { .symbol = borderColorSymbol, .value = styles.mainBorderColor },
      PropPair { .symbol = barColorSymbol, .value = styles.highlightColor },
    },
  };
  auto sliderWithProps = withPropsCopy(slider, sliderProps); 
  return sliderWithProps;
}