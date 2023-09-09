#include "./mainui.h"

extern CustomApiBindings* gameapi;

auto routerHistory = createHistory("mainmenu");

Props createLevelListProps(UiContext& uiContext){
  std::vector<ListComponentData> levels;
  auto levelDatas = uiContext.levels.getLevels();

  for (auto &levelData : levelDatas){
    levels.push_back(ListComponentData {
      .name = levelData.name,
      .onClick = [&uiContext, levelData]() -> void {
        auto level = levelData;
        uiContext.levels.goToLevel(level);
      }
    });
  }
  Props levelProps {
    .props = {
      PropPair { .symbol = listItemsSymbol, .value = levels },
      PropPair { .symbol = xoffsetSymbol,   .value = -0.9f },
      PropPair { .symbol = yoffsetSymbol,   .value = 0.2f },
      //PropPair { .symbol = tintSymbol,      .value = glm::vec4(0.f, 0.f, 0.f, 0.f) },
      PropPair { .symbol = flowHorizontal,  .value = UILayoutFlowPositive2 },
      PropPair { .symbol = flowVertical,    .value = UILayoutFlowNegative2 },
    },
  };
  return levelProps;
}

Props pauseMenuProps(std::optional<objid> mappingId, UiContext& uiContext){
  Props props {
    .props = {
      { .symbol = elapsedTimeSymbol, .value = uiContext.pauseInterface.elapsedTime },
      { .symbol = goToMainMenuSymbol, .value = uiContext.levels.goToMenu } ,
      { .symbol = resumeSymbol, .value = uiContext.pauseInterface.resume },
      { .symbol = yoffsetSymbol, .value = 0.2f },
    },
  };
  return props;
}

Props createRouterProps(UiContext& uiContext, std::optional<objid> selectedId){
  auto props = pauseMenuProps(selectedId, uiContext);
  auto pauseComponent = withPropsCopy(pauseMenuComponent, props);

  std::map<std::string, Component> routeToComponent = {
    { "mainmenu",  withPropsCopy(listComponent, createLevelListProps(uiContext)) },
    { "playing",  emptyComponent },
    { "paused", pauseComponent },
    { "",  emptyComponent  },
  };

  Props routerProps {
    .props = {
      { routerSymbol, routerHistory },
      { routerMappingSymbol, routeToComponent },
    },
  };
  return routerProps;
}

Props debugListProps { 
  .props = {
    PropPair {
      .symbol = tintSymbol,
      .value = glm::vec4(0.f, 0.f, 0.f, 0.8f),
    },
    PropPair {
      .symbol = minwidthSymbol,
      .value = 0.15f,
    },
    PropPair {
      .symbol = xoffsetSymbol,
      .value = -0.99f,
    },
    PropPair {
      .symbol = yoffsetSymbol,
      .value = 0.98f,  
    }
  }
};

std::string dockedDock = "";
std::function<void(const char*)> onClickNavbar = [](const char* value) -> void {
  //pushQueryParam(routerHistory, "dockedDock");
  dockedDock = value;
  modlog("dock", std::string("new dock: ") + dockedDock);
  if (dockedDock == ""){
    windowSetEnabled(windowDockSymbol, false);
  }else{
    windowSetEnabled(windowDockSymbol, true);
  }
};


std::string dialog = "";
std::function<void()> xCallbackFn = []() -> void {
  dialog = "";
};


bool showColorPicker = true;
std::function<void(glm::vec4)> onSlide = [](glm::vec4 value) -> void {
  static glm::vec4* activeColor = static_cast<glm::vec4*>(uiConnect(color));
  *activeColor = value;
};

std::function<void(bool closedWithoutNewFile, std::string file)> onFileAddedDefaultFn = [](bool closedWithoutNewFile, std::string file) -> void {
  modlog("dock", "on file added default");
};
std::function<void(bool closedWithoutNewFile, std::string file)> onFileAddedFn = onFileAddedDefaultFn;

DockConfigApi dockConfigApi { // probably should be done via a prop for better control flow
  .createCamera = []() -> void {
    modlog("dock", "mock make camera");
  },
  .createLight = []() -> void {
    modlog("dock", "mock make light");
  },
  .openFilePicker = [](std::function<void(bool closedWithoutNewFile, std::string file)> onFileAdded) -> void {
    windowSetEnabled(windowFileExplorerSymbol, true);
    onFileAddedFn = [onFileAdded](bool closedWithoutNewFile, std::string file) -> void {
      onFileAdded(closedWithoutNewFile, file);
      onFileAddedFn = onFileAddedDefaultFn;
      windowSetEnabled(windowFileExplorerSymbol, false);
    };
  },
  .openImagePicker = [](std::function<void(bool closedWithoutNewFile, std::string file)> onFileAdded) -> void {
    windowSetEnabled(windowImageExplorerSymbol, true);
    onFileAddedFn = [onFileAdded](bool closedWithoutNewFile, std::string file) -> void {
      onFileAdded(closedWithoutNewFile, file);
      onFileAddedFn = onFileAddedDefaultFn;
      windowSetEnabled(windowImageExplorerSymbol, false);
    };
  }
};

ImageList imageListDatas {
  .images = {},
};
int imageListScrollAmount = 0;

std::map<objid, std::function<void()>> handleDrawMainUi(UiContext& uiContext, std::optional<objid> selectedId){
  std::map<objid, std::function<void()>> handlerFns;
  DrawingTools drawTools {
     .drawText = gameapi -> drawText,
     .drawRect = gameapi -> drawRect,
     .drawLine2D = gameapi -> drawLine2D,
     .registerCallbackFns = [&handlerFns](objid id, std::function<void()> fn) -> void {
        handlerFns[id] = fn;
     },
     .selectedId = selectedId,
  };
  resetMenuItemMappingId();
  
  if (dockedDock != ""){
    Props dockProps { 
      .props = {
        { dockTypeSymbol, dockedDock }, 
        { xoffsetFromSymbol, 1.5f },
      }
    };
    auto dock = withProps(dockComponent, dockProps);
    auto defaultWindowProps = getDefaultProps();
    createUiWindow(dock, windowDockSymbol, dockedDock, AlignmentParams { .layoutFlowHorizontal = UILayoutFlowNegative2, .layoutFlowVertical = UILayoutFlowNegative2 }).draw(drawTools, defaultWindowProps);
  }


  std::vector<ListComponentData> dialogOptions = {
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

  if (dialog != ""){
    Props dialogProps {
      .props = {
        PropPair { .symbol = listItemsSymbol, .value = dialogOptions },
        PropPair { .symbol = titleSymbol, .value = std::string("mainui title") },
        PropPair { .symbol = detailSymbol, .value = std::string("mainui detail") },
        PropPair { .symbol = onclickSymbol, .value = xCallbackFn },
      },
    };
    dialogComponent.draw(drawTools, dialogProps);
  }

  {
    FileCallback onFileSelect = [](FileContentType type, std::string file) -> void {
      modassert(type == File, "on file select gave something not a file");
      modlog("dock - file select", std::string(type == Directory ? "dir" : "file") + " " + file);
      onFileAddedFn(false, file);
    };

    Props filexplorerProps {
      .props = {
        PropPair { .symbol = fileExplorerSymbol, .value = testExplorer },
        PropPair { .symbol = fileChangeSymbol, .value = onFileSelect }
      },
    };
    auto fileExplorer = withProps(fileexplorerComponent, filexplorerProps);
    auto fileExplorerWindow = createUiWindow(fileExplorer, windowFileExplorerSymbol, "File Explorer");
    auto defaultWindowProps = getDefaultProps();
    fileExplorerWindow.draw(drawTools, defaultWindowProps);
  }


  {
    static glm::vec4* activeColor = static_cast<glm::vec4*>(uiConnect(color));
    auto colorPicker = withPropsCopy(colorPickerComponent, Props {
      .props = { 
        PropPair { onSlideSymbol,  onSlide },
        PropPair { tintSymbol, *activeColor },
      }
    });
    auto uiWindowComponent = createUiWindow(colorPicker, windowColorPickerSymbol, "color picker");
    auto defaultWindowProps = getDefaultProps();
    uiWindowComponent.draw(drawTools, defaultWindowProps);
  }

  {


    std::function<void(int)> onImageClick = [](int index) -> void {
      onFileAddedFn(false, imageListDatas.images.at(index));
    };

    static bool loadedImages = false;
    if (!loadedImages){
      loadedImages = true;
      imageListDatas.images = gameapi -> listResources("textures");
    }

    auto imageListComponent = withPropsCopy(imageList, Props {
      .props = { 
        PropPair { imagesSymbol,  imageListDatas },
        PropPair { onclickSymbol, onImageClick },
        PropPair { offsetSymbol,  imageListScrollAmount },
      }
    });

    auto uiWindowComponent = createUiWindow(imageListComponent, windowImageExplorerSymbol, "Image Explorer");
    auto defaultWindowProps = getDefaultProps();
    uiWindowComponent.draw(drawTools, defaultWindowProps);
  }

  auto routerProps = createRouterProps(uiContext, selectedId);
  router.draw(drawTools, routerProps);

  if (uiContext.isDebugMode()){
    Props navbarProps { 
      .props = {
        { onclickSymbol, onClickNavbar }
      }
    };
    navbarComponent.draw(drawTools, navbarProps);

    auto defaultProps = getDefaultProps();
    withProps(debugList, debugListProps).draw(drawTools, defaultProps);
    drawTools.drawText(std::string("route: ") + routerHistory.currentPath, .8f, -0.95f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
    drawTools.drawText(std::string("handlers: ") + std::to_string(handlerFns.size()), .8f, -0.90f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
  }
  if (uiContext.showScreenspaceGrid()){
    drawScreenspaceGrid(ImGrid{ .numCells = 10 });
  }
  return handlerFns;
}

void onMainUiScroll(double amount){
  auto scrollValue = static_cast<int>(amount);
  std::cout << "dock: on main ui scroll: " << scrollValue << std::endl;
  imageListScrollAmount += (scrollValue * 5);
  if (imageListScrollAmount < 0){
    imageListScrollAmount = 0;
  }
}

void onMainUiMouseRelease(){
  windowOnRelease();
}

void pushHistory(std::string route){
	pushHistory(routerHistory, route);
}

std::string getCurrentPath(){
  return getCurrentPath(routerHistory);
}