#include "./dock_textbox_numeric.h"

Component createDockTextboxNumeric(DockTextboxNumeric& dockTextboxNumeric){
    // this is probably not the right type of display for this 
    Props textboxProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = std::string(dockTextboxNumeric.label) + "       " + std::to_string(dockTextboxNumeric.value) },
        PropPair { .symbol = paddingSymbol, .value = styles.dockElementPadding },
      }
    };
    auto textboxWithProps = withPropsCopy(textbox, textboxProps);
    return textboxWithProps;  
}