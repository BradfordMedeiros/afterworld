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
  std::cout << "navbar new dock: " << dockedDock << std::endl;
};


std::string dialog = "";
std::function<void()> xCallbackFn = []() -> void {
  dialog = "";
};

std::string fileexplorer = "files";
std::function<void()> xFileExplorerCallbackFn = []() -> void {
  fileexplorer = "";
};


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
        { titleSymbol, dockedDock },
        { xoffsetSymbol, 1.f },
        { yoffsetSymbol, 0.88f },
        { xoffsetFromSymbol, 1.5f },
      }
    };
    withAnimator(routerHistory, dockComponent, 0.1f).draw(drawTools, dockProps);
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

  if (fileexplorer != ""){
    Props filexplorerProps {
      .props = {
        PropPair { .symbol = listItemsSymbol, .value = dialogOptions },
        PropPair { .symbol = titleSymbol, .value = std::string("File Explorer") },
        PropPair { .symbol = onclickSymbol, .value = xFileExplorerCallbackFn },
      },
    };
    fileexplorerComponent.draw(drawTools, filexplorerProps);
  }

  auto defaultProps = getDefaultProps();
  auto routerProps = createRouterProps(uiContext, selectedId);
  router.draw(drawTools, routerProps);

  if (uiContext.isDebugMode()){
    Props navbarProps { 
      .props = {
        { onclickSymbol, onClickNavbar }
      }
    };
    navbarComponent.draw(drawTools, navbarProps);

    withProps(debugList, debugListProps).draw(drawTools, defaultProps);
    drawTools.drawText(std::string("route: ") + routerHistory.currentPath, .8f, -0.95f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
    drawTools.drawText(std::string("handlers: ") + std::to_string(handlerFns.size()), .8f, -0.90f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
  }
  if (uiContext.showScreenspaceGrid()){
    drawScreenspaceGrid(ImGrid{ .numCells = 10 });
  }
  return handlerFns;
}

void pushHistory(std::string route){
	pushHistory(routerHistory, route);
}

std::string getCurrentPath(){
  return getCurrentPath(routerHistory);
}