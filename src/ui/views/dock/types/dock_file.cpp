#include "./dock_file.h"

extern DockConfigApi dockConfigApi;

Component createDockFile(DockFileConfig& fileconfigOptions){
std::function<void()> onClick =  [&fileconfigOptions]() -> void {
    dockConfigApi.openFilePicker(
      [&fileconfigOptions](bool justClosed, std::string file) -> void {
        std::cout << "open file picker dialog mock: " << justClosed << ", file = " << file << std::endl;
        if (!justClosed){
          fileconfigOptions.label = file;
        }
      }, 
      [](bool isDirectory, std::string&) -> bool { 
        return true; 
      });
  };

  std::string fileDisplay = fileconfigOptions.label;
  if (fileconfigOptions.displayLimit.has_value()){
    int firstIndex = fileDisplay.size() - fileconfigOptions.displayLimit.value();
    if (firstIndex < 0){
      firstIndex = 0;
    }
    fileDisplay = "[...] " + fileDisplay.substr(firstIndex, fileDisplay.size()) ; 
  }
  Props textboxProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = fileDisplay },
      PropPair { .symbol = onclickSymbol, .value = onClick },
    }
  };
  auto textboxWithProps = withPropsCopy(textbox, textboxProps);
  return textboxWithProps; 
}