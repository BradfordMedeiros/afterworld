#include "./dock_select.h"

SelectOptions selectOptions {
  .options = { "one", "two", "three", "four" },
  .toggleExpanded = [](bool expanded) -> void {
    selectOptions.isExpanded = expanded;
  },
  .onSelect = [](int index, std::string&) -> void {
    selectOptions.currentSelection = index;
    selectOptions.isExpanded = false;
  },
  .currentSelection = 0,
  .isExpanded = false,
};


Component createDockSelect(DockSelectConfig& dockSelect){

  Props selectProps {
     .props = {
       PropPair { .symbol = valueSymbol, .value = selectOptions },
     }
  };
  return withPropsCopy(selectComponent, selectProps);
}