#include "./router.h"

extern CustomApiBindings* gameapi;

std::optional<std::function<void()>> registerOnRouteChangedFn;
RouterHistory createHistory(std::string initialRoute){
	return RouterHistory {
		.currentPath = initialRoute,
    .initialRoute = initialRoute,
    .currentRouteTime = 0.f,
    .history = {},
	};
}
void pushHistory(RouterHistory& history, std::string path, bool replace){
  history.currentPath = path;
  history.currentRouteTime = gameapi -> timeSeconds(true);
  if (replace){
    history.history = {};
  }
  history.history.push_back(path);
  if (registerOnRouteChangedFn.has_value()){
    registerOnRouteChangedFn.value()();
  }
}

void popHistory(RouterHistory& history){
  if (history.history.size() == 0){
    return;
  }
  history.history.pop_back();
  if (history.history.size() == 0){
    pushHistory(history, history.initialRoute, false);
  }else{
    history.currentPath = history.history.back();
    history.currentRouteTime = gameapi -> timeSeconds(true);
    if (registerOnRouteChangedFn.has_value()){
      registerOnRouteChangedFn.value()();
    }
  }
}

std::string fullHistoryStr(RouterHistory& history){
  std::string str = "";
  for (auto &path : history.history){
    str += path + "/";
  }
  return str;
}

std::string getCurrentPath(RouterHistory& history){
  return history.currentPath;
}

RouterHistory* routerHistory(Props& props){
 	auto propPair = propPairAtIndex(props.props, routerSymbol);
 	if (!propPair){
 		return NULL;
 	}
 	RouterHistory* router = anycast<RouterHistory>(propPair -> value);
 	modassert(router, "invalid router propr");
 	return router;
}

std::map<std::string, Component>* routerMapping(Props& props){
  auto propPair = propPairAtIndex(props.props, routerMappingSymbol);
  if (!propPair){
    return NULL;
  }
  std::map<std::string, Component>* routerMapping = anycast<std::map<std::string, Component>>(propPair -> value);
  modassert(routerMapping, "invalid router mapping");
  return routerMapping;
}


const Component* componentAtRoute(const std::map<std::string, Component>& routeToComponent, std::string& path){
  auto hasRoute = routeToComponent.find(path) != routeToComponent.end();
  if (!hasRoute){
    return NULL;
  }
  return &routeToComponent.at(path);
}

Component router {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
  	auto history = routerHistory(props);
  	if (!history){
  		modlog("router", "no history");
	     return BoundingBox2D { .x = 0.f, .y = 0.f, .width = 0.f, .height = 0.f };    		
  	}
    auto routeToComponent = routerMapping(props);
    modassert(routeToComponent, "router - no router mapping");

    auto fullPath = fullHistoryStr(*history);
    auto component = componentAtRoute(*routeToComponent, fullPath);
  	if (!component){
  		modlog("router", std::string("no path for: ") + fullPath);
  		return BoundingBox2D { .x = 0.f, .y = 0.f, .width = 0.f, .height = 0.f };
  	}
    return component -> draw(drawTools, props);
  },
};

Component withAnimator(RouterHistory& history, Component& component, float duration){
  float elapsedTime = gameapi -> timeSeconds(true) - history.currentRouteTime;
  float interpAmount = glm::min(1.f, elapsedTime / duration);

  Props props {
    .props = {
      PropPair  { .symbol = interpolationSymbol, .value = interpAmount },
    },
  };
  return withPropsCopy(component, props);
}

void registerOnRouteChanged(std::function<void()> onRouteChanged){
  modassert(!registerOnRouteChangedFn.has_value(), "can only register a single route");
  registerOnRouteChangedFn = onRouteChanged;
}