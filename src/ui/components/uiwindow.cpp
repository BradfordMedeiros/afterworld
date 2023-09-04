#include "./uiwindow.h"

Component createUiWindow(std::vector<Component>& components, std::string& titleValue, std::function<void()> onClick){
  auto titleTextbox = withPropsCopy(listItem, Props {
    .props = {
      PropPair { .symbol = valueSymbol, .value = titleValue },
      PropPair { .symbol = onclickSymbol, .value = onClick },
    }
  });

  std::vector<Component> allComponents;
  allComponents.push_back(titleTextbox);

  for (auto &component : components){
    allComponents.push_back(component);
  }
  return simpleVerticalLayout(allComponents);
}
