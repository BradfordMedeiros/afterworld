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
      PropPair { .symbol = tintSymbol,      .value = glm::vec4(0.f, 0.f, 0.f, 0.f) },
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
      { tintSymbol, glm::vec4(1.f, 1.f, 1.f, 0.2f) }
    },
  };
  return routerProps;
}

Props nestedListProps2 { 
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
      .value = -1.f,
    },
    PropPair {
      .symbol = yoffsetSymbol,
      .value = 0.98f,  
    }
  }
};

std::string dockedDock = "";
std::function<void(const char*)> onClickNavbar = [](const char* value) -> void {
  dockedDock = value;
  std::cout << "navbar new dock: " << dockedDock << std::endl;
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


  //Props navbarProps { 
  //  .props = {
  //    { onclickSymbol, onClickNavbar }
  //  }
  //};
  //navbarComponent.draw(drawTools, navbarProps);
  //
  //if (dockedDock != ""){
  //  Props dockProps { 
  //    .props = {
  //      { titleSymbol, dockedDock },
  //      { yoffsetSymbol, 0.f },
  //    }
  //  };
  //  dockComponent.draw(drawTools, dockProps);
  //}
//
//
  auto defaultProps = getDefaultProps();
//  //auto routerProps = createRouterProps(uiContext, selectedId);
  //router.draw(drawTools, routerProps);


  /////////////

  std::vector<Component> elements;
  Props listItemProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = std::string("testvalue") },
      PropPair { .symbol = onclickSymbol, .value = nullClick.value() },
      PropPair { .symbol = paddingSymbol, .value = 0.125f },
      PropPair { .symbol = tintSymbol, .value = glm::vec4(0.f, 1.f, 0.f, 1.f) },
    },
  };
  auto listItemWithProps = withPropsCopy(listItem, listItemProps);
  elements.push_back(listItemWithProps);

  Props listItemProps2 {
    .props = {
      PropPair { .symbol = valueSymbol, .value = std::string("testvalue") },
      PropPair { .symbol = onclickSymbol, .value = nullClick.value() },
      PropPair { .symbol = paddingSymbol, .value = 0.25f },
      PropPair { .symbol = tintSymbol, .value = glm::vec4(1.f, 0.f, 0.f, 1.f) },
    },
  };
  auto listItemWithProps2 = withPropsCopy(listItem, listItemProps2);
  elements.push_back(listItemWithProps2);

  Layout layout {
    .tint = glm::vec4(1.f, 0.f, 0.f, 0.8f),
    .showBackpanel = true,
    .borderColor = glm::vec4(0.f, 1.f, 1.f, 1.f),
    .minwidth = 0.f,
    .minheight = 0.f,
    .layoutType = LAYOUT_HORIZONTAL2,
    .layoutFlowHorizontal = UILayoutFlowNone2,
    .layoutFlowVertical = UILayoutFlowNone2,
    .alignHorizontal = UILayoutFlowNone2,
    .alignVertical = UILayoutFlowNone2,
    .spacing = 0.f,
    .minspacing = 0.f,
    .padding = 0.02f,
    .children = elements,
  };

  Props listLayoutProps {
    .props = {
      { .symbol = layoutSymbol, .value = layout },
    },
  };
  std::cout << "layout start" << std::endl;
  layoutComponent.draw(drawTools, listLayoutProps);
  std::cout << "layout end" << std::endl << std::endl;

  //////////////////

  if (uiContext.isDebugMode()){
    withProps(nestedListTestComponent, nestedListProps2).draw(drawTools, defaultProps);

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