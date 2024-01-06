#include "./dock_select.h"

Component createDockSelect(DockSelectConfig& dockSelect){

  Props selectProps {
     .props = {
       PropPair { .symbol = valueSymbol, .value = dockSelect.selectOptions },
     }
  };
  return withPropsCopy(selectComponent, selectProps);
}