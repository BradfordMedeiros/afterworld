#include "./dock_label.h"

Component createDockLabel(DockLabelConfig& dockLabel){
    Props textboxProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = dockLabel.label() },
      }
    };
    auto textboxWithProps = withPropsCopy(textbox, textboxProps);
    return textboxWithProps;  
}