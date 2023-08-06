#include "./router.h"

void pushHistory(RouterHistory& history, std::string path){
	history.currentPath = path;
}

RouterHistory createHistory(std::string initialRoute){
	return RouterHistory {
		.currentPath = initialRoute,
	};
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
    	if (routeToComponent.find(history -> currentPath) == routeToComponent.end()){
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