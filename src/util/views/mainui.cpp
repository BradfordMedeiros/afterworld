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

void handleDrawMainUi(DrawingTools& drawTools, std::optional<objid> selectedId){
   Props routerProps {
     .mappingId = selectedId,
     .props = {
       { routerSymbol, routerHistory },
       { tintSymbol, glm::vec4(1.f, 1.f, 1.f, 0.2f) }
     },
   };
   mainUI.draw(drawTools, routerProps);
}

void handleInputMainUi(std::optional<objid> selectedId){
   Props routerProps {
     .mappingId = selectedId,
     .props = {
       { routerSymbol, routerHistory },
     },
   };
   mainUI.imMouseSelect(selectedId, routerProps);
}

void pushHistory(std::string route){
	pushHistory(routerHistory, route);
}

//