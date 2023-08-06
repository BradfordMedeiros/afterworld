#include "./router.h"

RouterHistory createHistory(std::string initialRoute){
	return RouterHistory {
		.currentPath = initialRoute,
	};
}
void pushHistory(RouterHistory& history, std::string path){
  history.currentPath = path;
}

const int routerSymbol = getSymbol("router");
RouterHistory* routerHistory(Props& props){
 	auto propPair = propPairAtIndex(props.props, routerSymbol);
 	if (!propPair){
 		return NULL;
 	}
 	RouterHistory* router = anycast<RouterHistory>(propPair -> value);
 	modassert(router, "invalid router propr");
 	return router;
}


Component createRouter(std::map<std::string, Component> routeToComponent){
  auto component = Component {
    .draw = [routeToComponent](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    	auto history = routerHistory(props);
    	if (!history){
    		modlog("router", "no history");
	      return BoundingBox2D { .x = 0.f, .y = 0.f, .width = 0.f, .height = 0.f };    		
    	}

      auto hasRoute = routeToComponent.find(history -> currentPath) != routeToComponent.end();
      drawTools.drawText(std::string("route: ") + history -> currentPath, .8f, -0.95f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
    	if (!hasRoute){
    		modlog("router", std::string("no path for: ") + history -> currentPath);
    		return BoundingBox2D { .x = 0.f, .y = 0.f, .width = 0.f, .height = 0.f };
    	}
      return routeToComponent.at(history -> currentPath).draw(drawTools, props);
    },
    .imMouseSelect = [](std::optional<objid> mappingIdSelected) -> void {
    },
  };
  return component;
}