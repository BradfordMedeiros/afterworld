#include "./dock_colorpicker.h"

extern DockConfigApi dockConfigApi;

Component createDockColorPicker(DockColorPickerConfig& dockColorPicker){
	std::function<void()> onClick = [&dockColorPicker]() -> void {
		dockConfigApi.openColorPicker(dockColorPicker.onColor);
	};
  Props listItemProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = dockColorPicker.label },
      PropPair { .symbol = tintSymbol, .value = dockColorPicker.getColor() },
      PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.8f) },
      PropPair { .symbol = paddingSymbol, .value = styles.dockElementPadding },
      PropPair { .symbol = onclickSymbol, .value = onClick },
      PropPair { .symbol = minwidthSymbol, .value = 0.5f },
      PropPair { .symbol = borderColorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 0.1f) },
      //PropPair { .symbol = fontsizeSymbol, .value = 0.03f }
    },
  };
  auto listItemWithProps = withPropsCopy(listItem, listItemProps);
  return listItemWithProps;
}