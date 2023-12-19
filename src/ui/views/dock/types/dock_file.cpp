#include "./dock_file.h"

extern DockConfigApi dockConfigApi;

Component createDockFile(DockFileConfig& fileconfigOptions){
std::function<void()> onClick =  [&fileconfigOptions]() -> void {
    dockConfigApi.openFilePicker([&fileconfigOptions](bool justClosed, std::string file) -> void {
      std::cout << "open file picker dialog mock: " << justClosed << ", file = " << file << std::endl;
      if (!justClosed){
        fileconfigOptions.label = file;
      }
    }, [](bool isDirectory, std::string&) -> bool { return !isDirectory; });
  };
  Props textboxProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = fileconfigOptions.label },
      PropPair { .symbol = onclickSymbol, .value = onClick },
    }
  };
  auto textboxWithProps = withPropsCopy(textbox, textboxProps);
  return textboxWithProps; 
}