#include "./dock_checkbox.h"

Component createDockCheckbox(DockCheckboxConfig& checkboxOptions){
  Props checkboxProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = checkboxOptions.label },
      PropPair { .symbol = checkedSymbol, .value = checkboxOptions.isChecked() },
      PropPair { .symbol = onclickSymbol, .value = checkboxOptions.onChecked },
    },
  };
  auto checkboxWithProps = withPropsCopy(checkbox, checkboxProps);
  return checkboxWithProps;
}