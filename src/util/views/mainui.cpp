#include "./mainui.h"

extern CustomApiBindings* gameapi;

const int routerSymbol = getSymbol("router");
const int routerMappingSymbol = getSymbol("router-mapping");
auto routerHistory = createHistory("mainmenu");

const int tintSymbol = getSymbol("tint");
const int listItemsSymbol = getSymbol("listitems");
const int minwidthSymbol = getSymbol("minwidth");
const int xoffsetSymbol = getSymbol("xoffset");
const int yoffsetSymbol = getSymbol("yoffset");
const int radioSymbol  = getSymbol("radio");
const int elapsedTimeSymbol = getSymbol("elapsedTime");
const int pausedSymbol = getSymbol("pause");
const int resumeSymbol = getSymbol("resume");


Props createLevelListProps(UiContext& uiContext){
  std::vector<ListComponentData> levels;
  auto levelDatas = uiContext.levels.getLevels();

  for (auto &levelData : levelDatas){
    levels.push_back(ListComponentData {
      .name = levelData.name,
      .onClick = [&uiContext, levelData]() -> void {
        auto level = levelData;
        uiContext.levels.goToLevel(level);
        uiContext.pauseInterface.pause();
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
      { .symbol = pausedSymbol, .value = uiContext.pauseInterface.pause } ,
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
      .value = -0.99f,
    },
    PropPair {
      .symbol = yoffsetSymbol,
      .value = 0.98f,  
    }
  }
};
std::map<objid, std::function<void()>> handleDrawMainUi(UiContext& uiContext, std::optional<objid> selectedId){
  std::map<objid, std::function<void()>> handlerFns;
  DrawingTools drawTools {
     .drawText = gameapi -> drawText,
     .drawRect = gameapi -> drawRect,
     .drawLine2D = gameapi -> drawLine2D,
     .registerCallbackFns = [&handlerFns](objid id, std::function<void()> fn) -> void {
        handlerFns[id] = [id, fn]() -> void {
          std::cout << std::string("test handler fn for : ") + std::to_string(id) << std::endl;
          fn();
        };
     },
     .selectedId = selectedId,
  };
  resetMenuItemMappingId();

  auto defaultProps = getDefaultProps(selectedId);
  auto routerProps = createRouterProps(uiContext, selectedId);
  router.draw(drawTools, routerProps);
  withProps(nestedListTestComponent, nestedListProps2).draw(drawTools, defaultProps);

  if (uiContext.showScreenspaceGrid()){
    drawScreenspaceGrid(ImGrid{ .numCells = 10 });
  }
  return handlerFns;
}

void pushHistory(std::string route){
	pushHistory(routerHistory, route);
}

