#include "./mainui.h"

const int routerSymbol = getSymbol("router");
auto routerHistory = createHistory("playing");

const int tintSymbol = getSymbol("tint");

Component mainUI = createRouter({
  { "playing",  createList(
    { ListComponentData { .name = "playing", .onClick = []() -> void {
        pushHistory("paused");
      }},
      ListComponentData { .name = "playing", .onClick = []() -> void {
        pushHistory("quit");
      }}
    }
  )},
  { "paused",   createList({
    { ListComponentData { .name = "paused", .onClick = std::nullopt }}
  })  },
  { "quit",  createList(
    { ListComponentData { .name = "quit", .onClick = []() -> void {
      exit(0);
    }}}
  )},
  { "",   createList({
    { ListComponentData { .name = "default", .onClick = std::nullopt }}
  })  },
});


Props pauseMenuProps(std::optional<objid> mappingId, PauseContext pauseContext){
  Props props {
    .mappingId = mappingId,
    .props = {
      { .symbol = getSymbol("elapsedTime"), .value = pauseContext.elapsedTime },
      { .symbol = getSymbol("pause"), .value = pauseContext.pause } ,
      { .symbol = getSymbol("resume"), .value = pauseContext.resume },
      { .symbol = getSymbol("yoffset"), .value = 0.2f },
    },
  };
  return props;
}

void handleDrawMainUi(PauseContext& pauseContext, DrawingTools& drawTools, std::optional<objid> selectedId){
   Props routerProps {
     .mappingId = selectedId,
     .props = {
       { routerSymbol, routerHistory },
       { tintSymbol, glm::vec4(1.f, 1.f, 1.f, 0.2f) }
     },
   };
   mainUI.draw(drawTools, routerProps);

   Props nestedListProps { 
    .mappingId = selectedId, 
    .props = {
      PropPair {
        .symbol = getSymbol("tint"),
        .value = glm::vec4(0.f, 0.f, 0.f, 0.8f),
      },
      PropPair {
        .symbol = getSymbol("minwidth"),
        .value = 0.15f,
      },
      PropPair {
        .symbol = getSymbol("xoffset"),
        .value = -0.99f,
      },
      PropPair {
        .symbol = getSymbol("yoffset"),
        .value = 0.98f,  
      }
    }
  };
  nestedListTestComponent.draw(drawTools, nestedListProps);

  if (pauseContext.shouldShowPauseMenu){
    auto props = pauseMenuProps(selectedId, pauseContext);
    createPauseMenuComponent().draw(drawTools, props);    
  }

    /*drawDebugBoundingBox(*/ //);

}

void handleInputMainUi(PauseContext& pauseContext, std::optional<objid> selectedId){
   Props routerProps {
     .mappingId = selectedId,
     .props = {
       { routerSymbol, routerHistory },
     },
   };
   mainUI.imMouseSelect(selectedId, routerProps);

  Props nestedListProps { 
    .mappingId = selectedId, 
    .props = {}
  };
  nestedListTestComponent.imMouseSelect(selectedId, nestedListProps);
}

void pushHistory(std::string route){
	pushHistory(routerHistory, route);
}

//