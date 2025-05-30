#include "./fileexplorer.h"

// TODO - this depends upon the real files on the system

const float UI_STYLE_EXPLORER_BUTTON_PADDING = 0.02f;

const int fileExplorerSymbol = getSymbol("file-explorer");
const int fileChangeSymbol = getSymbol("file-change");
const int fileFilterSymbol = getSymbol("filter-filter");

typedef std::function<void(std::string)> ExplorerNavigation;

std::vector<ListComponentData> fileExplorerOptions = {
  ListComponentData {
    .name = "confirm",
    .onClick = []() -> void {
      std::cout << "dialog confirm on click" << std::endl;
    },      
  },
  ListComponentData {
    .name = "quit",
    .onClick = []() -> void {
      std::cout << "dialog on quit" << std::endl;
      exit(1);
    },      
  },
};


Component fileexplorerComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto fileExplorer = typeFromProps<FileExplorer>(props, fileExplorerSymbol);
    modassert(fileExplorer, "fileexplorer must be defined for dialog");
 
    auto customFileCallbackPtr = typeFromProps<FileCallback>(props, fileChangeSymbol);
    modassert(customFileCallbackPtr, "fileexplorer no custom file callback defined");
    auto customFileCallback = *customFileCallbackPtr;

    auto fileFilterPtr = typeFromProps<FileFilter>(props, fileFilterSymbol);

    std::vector<Component> elements;
    auto onChange = fileExplorer -> explorerOnChange;
    // current path
    { 
      std::vector<Component> pathElements;
      int startIndex = fileExplorer -> currentPath.size() - 4;
      if (startIndex < 0){
        startIndex = 0;
      }
      for (int i = startIndex ; i < fileExplorer -> currentPath.size(); i++){
        auto value = fileExplorer -> currentPath.at(i);

        auto clickedPath = std::string("/") + join(subvector(fileExplorer -> currentPath, 0, i + 1), '/');
        std::function<void()> onClick = [clickedPath, onChange]() -> void {
          onChange(true, clickedPath);
        };
        Props listItemProps {
          .props = {
            PropPair { .symbol = valueSymbol, .value = value },
            PropPair { .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 0.f, 1.f) },
            PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.8f) },
            PropPair { .symbol = paddingSymbol, .value = 0.01f },
            PropPair { .symbol = onclickSymbol, .value = onClick },
          },
        };
        auto listItemWithProps = withPropsCopy(listItem, listItemProps);
        pathElements.push_back(listItemWithProps);
      }
      auto pathComponent = simpleHorizontalLayout(pathElements);
      elements.push_back(pathComponent);
      modassert(elements.size() > 0, "need at least 1 elements in path for fileExplorer");
    }
    /////////////////////
    // files
    {
      std::vector<Component> pathElements;
      int numElementsDisplayed = 0;
      int offset = intFromProp(props, offsetSymbol, 0);

      for (int i = offset; (i < fileExplorer -> currentContents.size() && numElementsDisplayed < 10); i++){
        auto value = fileExplorer -> currentContents.at(i);
        auto shouldShow = fileFilterPtr ? (*fileFilterPtr)(value.isDirectory, value.content) : true;
        if (!shouldShow){
          continue;
        }
        auto isDirectory = value.isDirectory;
        auto clickedPathVec = fileExplorer -> currentPath;
        clickedPathVec.push_back(value.content);
        auto clickedPath = join(clickedPathVec, '/');
   
        std::function<void()> onClick = [clickedPath, onChange, isDirectory, customFileCallback]() -> void {
          std::cout << "on click: " <<  clickedPath << std::endl;
          onChange(isDirectory, clickedPath);
          if (!isDirectory){
            customFileCallback(isDirectory, clickedPath);
          }
        };

        Props listItemProps {
          .props = {
            PropPair { .symbol = valueSymbol, .value = (value.isDirectory ? std::string("[D] " + value.content) : value.content) },
            PropPair { .symbol = tintSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.f) },
            PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.8f) },
            PropPair { .symbol = paddingSymbol, .value = 0.01f },
            PropPair { .symbol = onclickSymbol, .value = onClick },
          },
        };
        auto listItemWithProps = withPropsCopy(listItem, listItemProps);
        pathElements.push_back(listItemWithProps);
        numElementsDisplayed++;
      }

      if (pathElements.size() == 0){
        Props listItemProps {
          .props = {
            PropPair { .symbol = valueSymbol, .value = std::string("no data") },
            PropPair { .symbol = tintSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.f) },
            PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.8f) },
            PropPair { .symbol = paddingSymbol, .value = 0.01f },
          },
        };
        auto listItemWithProps = withPropsCopy(listItem, listItemProps);
        pathElements.push_back(listItemWithProps);  
      }
      auto pathComponent = simpleVerticalLayout(pathElements, glm::vec2(0.5f, 0.5f), AlignmentParams {
        .layoutFlowHorizontal = UILayoutFlowNone2,
        .layoutFlowVertical = UILayoutFlowNegative2,
      }, glm::vec4(0.f, 0.f, 1.f, 1.0f), 0.f, glm::vec4(0.f, 0.f, 1.f, 0.5f));
      elements.push_back(pathComponent);        
      
      modassert(elements.size() > 0, "need at least 1 elements in files for fileExplorer");
    }
    ///////////////

    {
      std::vector<Component> choiceElements;
      for (int i = 0 ; i < fileExplorerOptions.size(); i++){
        ListComponentData& listItemData = fileExplorerOptions.at(i);
        auto onClick = listItemData.onClick.has_value() ? listItemData.onClick.value() : []() -> void {};
        Props listItemProps {
          .props = {
            PropPair { .symbol = valueSymbol, .value = listItemData.name },
            PropPair { .symbol = onclickSymbol, .value = onClick },
            PropPair { .symbol = tintSymbol, .value = glm::vec4(0.2f, 0.2f, 0.2f, 0.2f) },
            PropPair { .symbol = paddingSymbol, .value = UI_STYLE_EXPLORER_BUTTON_PADDING },
          },
        };
        auto listItemWithProps = withPropsCopy(listItem, listItemProps);
        choiceElements.push_back(listItemWithProps);
      }
      auto horizontalComponent = simpleHorizontalLayout(choiceElements);
      elements.push_back(horizontalComponent);
    }
    /////////////////////////

    return simpleVerticalLayout(elements, glm::vec2(0.f, 0.f), defaultAlignment, glm::vec4(1.f, 1.f, 1.f, 0.4f), 0.f, glm::vec4(0.f, 0.f, 0.f, 0.8f)).draw(drawTools, props);
  },
};


struct FileNavigator {
  std::filesystem::path path;
};

FileNavigator createFileNavigator(){
  auto path = std::filesystem::path(".");
  auto navigator = FileNavigator{
    .path = path,
  };
  return navigator;
}
void navigateDir(FileNavigator& navigator, std::string directory){
  auto path = std::filesystem::weakly_canonical(std::filesystem::path(std::filesystem::absolute(directory)));
  navigator.path = path;
  modlog("navigator", std::string("current path: ") +  std::string(std::filesystem::absolute(navigator.path)));
}
std::vector<std::string> getCurrentNavigatorPath(FileNavigator& navigator){
  std::string resourcePath = std::filesystem::absolute(navigator.path);
  auto currentPath = split(resourcePath, '/');
  modlog("current path: ", print(currentPath));
  modassert(currentPath.size() > 0, "current path must be > 0");
  return currentPath;
}
std::vector<FileContent> listCurrentContents(FileNavigator& navigator){
  std::vector<FileContent> files;
  for(auto &file: std::filesystem::directory_iterator(navigator.path)) {
    if (!std::filesystem::is_directory(file)) {
      files.push_back(FileContent {
        .isDirectory = false,
        .content = std::filesystem::path(file.path()).filename(),
      });
    }else{
      files.push_back(FileContent {
        .isDirectory = true,
        .content = std::filesystem::path(file.path()).filename(),
      });
    }
  }
  return files;
}

FileNavigator navigator = createFileNavigator();

FileExplorer testExplorer {
  .currentPath = getCurrentNavigatorPath(navigator),
  .currentContents  = listCurrentContents(navigator),
  .explorerOnChange = [](bool isDirectory, std::string file) -> void {
    if (!isDirectory){
      std::cout << "navigator explorer on change - file: " << file << std::endl;
    }else {
      std::cout << "navigator explorer on change - directory: " << file << std::endl;
      navigateDir(navigator, file);
      testExplorer.currentContents = listCurrentContents(navigator);
      testExplorer.currentPath = getCurrentNavigatorPath(navigator);
    }
  }
};