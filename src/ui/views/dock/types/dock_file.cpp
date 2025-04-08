#include "./dock_file.h"

extern DockConfigApi dockConfigApi;

std::string limitDisplayFilePath(std::string& label, std::optional<int> displayLimit){
  std::string fileDisplay = label;
  if (displayLimit.has_value()){
    int firstIndex = fileDisplay.size() - displayLimit.value();
    if (firstIndex < 0){
      firstIndex = 0;
    }
    fileDisplay = "[...] " + fileDisplay.substr(firstIndex, fileDisplay.size()) ; 
  }
  return fileDisplay;
}

Component createDockFile(DockFileConfig& fileconfigOptions){
std::function<void()> onClick =  [&fileconfigOptions]() -> void {
    dockConfigApi.openFilePicker(
      [&fileconfigOptions](bool justClosed, std::string file) -> void {
        std::cout << "open file picker dialog mock: " << justClosed << ", file = " << file << std::endl;
        if (!justClosed){
          fileconfigOptions.label = file;
          fileconfigOptions.onFileSelected(file);
        }
      }, 
      [](bool isDirectory, std::string&) -> bool { 
        return true; 
      });
  };

  auto fileDisplay = limitDisplayFilePath(fileconfigOptions.label, fileconfigOptions.displayLimit);

  Props textboxProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = fileDisplay },
      PropPair { .symbol = onclickSymbol, .value = onClick },
    }
  };
  auto textboxWithProps = withPropsCopy(textbox, textboxProps);
  return textboxWithProps; 
}