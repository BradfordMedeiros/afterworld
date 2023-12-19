#include "./dock_gameobj.h"

extern DockConfigApi dockConfigApi;

Component createDockGameobj(DockGameObjSelector& gameobjSelectorOptions){
  std::function<void()> onClick =  [&gameobjSelectorOptions]() -> void {
    gameobjSelectorOptions.label = std::nullopt;
    dockConfigApi.pickGameObj([&gameobjSelectorOptions](objid id, std::string value) -> void {
      std::cout << "dock gameobjSelectorOptions onclick:  " << id << ", " << value << std::endl;
      gameobjSelectorOptions.label = value;
      gameobjSelectorOptions.onSelect(value);
    });
  };
  std::string value = gameobjSelectorOptions.label.has_value() ? (std::string("target: ") + gameobjSelectorOptions.label.value()) : std::string("none");
  Props textboxProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = value },
      PropPair { .symbol = onclickSymbol, .value = onClick },
      PropPair { .symbol = paddingSymbol, .value = styles.dockElementPadding },
    }
  };
  if (!gameobjSelectorOptions.label.has_value()){
    textboxProps.props.push_back(PropPair {
      .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 1.f, 1.f),
    });
  }
  auto textboxWithProps = withPropsCopy(listItem, textboxProps);
  return textboxWithProps; 	
}