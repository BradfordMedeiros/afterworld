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

const int routerMappingSymbol = getSymbol("router-mapping");
std::map<std::string, Component>* routerMapping(Props& props){
  auto propPair = propPairAtIndex(props.props, routerMappingSymbol);
  if (!propPair){
    return NULL;
  }
  std::map<std::string, Component>* routerMapping = anycast<std::map<std::string, Component>>(propPair -> value);
  modassert(routerMapping, "invalid router mapping");
  return routerMapping;
}


const Component* componentAtRoute(const std::map<std::string, Component>& routeToComponent, RouterHistory& history){
  auto hasRoute = routeToComponent.find(history.currentPath) != routeToComponent.end();
  if (!hasRoute){
    return NULL;
  }
  return &routeToComponent.at(history.currentPath);
}

Component createRouter(){
  auto component = Component {
    .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    	auto history = routerHistory(props);
    	if (!history){
    		modlog("router", "no history");
	      return BoundingBox2D { .x = 0.f, .y = 0.f, .width = 0.f, .height = 0.f };    		
    	}
      auto routeToComponent = routerMapping(props);
      modassert(routeToComponent, "router - no router mapping");
      auto component = componentAtRoute(*routeToComponent, *history);
      drawTools.drawText(std::string("route: ") + history -> currentPath, .8f, -0.95f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
    	if (!component){
    		modlog("router", std::string("no path for: ") + history -> currentPath);
    		return BoundingBox2D { .x = 0.f, .y = 0.f, .width = 0.f, .height = 0.f };
    	}
      return component -> draw(drawTools, props);
    },
    .imMouseSelect = [](std::optional<objid> mappingId, Props& props) -> void {
      auto history = routerHistory(props);
      if (!history){
        return;
      }
      auto routeToComponent = routerMapping(props);
      modassert(routeToComponent, "router - no router mapping");
      auto component = componentAtRoute(*routeToComponent, *history);
      if (component){
        component -> imMouseSelect(mappingId, props);
      }  
    },
  };
  return component;
}