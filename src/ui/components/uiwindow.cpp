#include "./uiwindow.h"

Component createUiWindow(std::vector<Component>& components, std::string& titleValue){
  std::function<void()> onWindowDrag = []() -> void {
    std::cout << "on window drag" << std::endl;
    exit(1);
  };
  auto titleTextbox = withPropsCopy(listItem, Props {
    .props = {
      PropPair { .symbol = valueSymbol, .value = titleValue },
      PropPair { .symbol = onclickSymbol, .value = onWindowDrag },
    }
  });

  std::vector<Component> allComponents;
  allComponents.push_back(titleTextbox);

  for (auto &component : components){
    allComponents.push_back(component);
  }
  return simpleVerticalLayout(allComponents);
}
