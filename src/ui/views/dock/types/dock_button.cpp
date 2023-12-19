#include "./dock_button.h"

Component createDockButton(DockButtonConfig& dockButton){
   Props buttonProps {
     .props = {
       PropPair { .symbol = paddingSymbol, .value = styles.dockElementPadding },
       PropPair { .symbol = valueSymbol, .value = std::string(dockButton.buttonText) },
       PropPair { .symbol = onclickSymbol, .value =  dockButton.onClick }
     }
   };
   return withPropsCopy(button, buttonProps);	
}
