#include "./dock_options.h"

Component createDockOptions(DockOptionConfig& dockOptions){
   std::vector<Option> optionsData = {};
   for (int i = 0; i < dockOptions.options.size(); i++){
     auto option = dockOptions.options.at(i);
     auto onClick = dockOptions.onClick;
     optionsData.push_back(Option {
       .name = option,
       .onClick = [onClick, option, i]() -> void {
         std::string optionStr(option);
         onClick(optionStr, i);
       },
     });
   }
   Options defaultOptions {
     .options = optionsData,
     .selectedIndex = dockOptions.getSelectedIndex(),
   };
   Props optionsProps {
     .props = {
       PropPair { .symbol = optionsSymbol, .value = defaultOptions },
       PropPair { .symbol = itemPaddingSymbol, .value = styles.dockElementPadding },
     }
   };
   return withPropsCopy(options, optionsProps);
}