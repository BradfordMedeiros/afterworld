#include "./dock_image.h"

extern DockConfigApi dockConfigApi;

Component createDockImage(DockImageConfig& imageConfigOptions){
  std::function<void()> onClick =  [&imageConfigOptions]() -> void {
    dockConfigApi.openImagePicker([&imageConfigOptions](bool justClosed, std::string image) -> void {
      if (!justClosed){
        imageConfigOptions.label = image;
        imageConfigOptions.onImageSelect(image);
      }
    });
  };
  Props textboxProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = imageConfigOptions.label },
      PropPair { .symbol = onclickSymbol, .value = onClick },
    }
  };
  auto textboxWithProps = withPropsCopy(textbox, textboxProps);
  return textboxWithProps; 
}