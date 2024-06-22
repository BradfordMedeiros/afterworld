#include "./router.h"

extern CustomApiBindings* gameapi;

std::optional<std::function<void()>> registerOnRouteChangedFn;
RouterHistory createHistory(){
	return RouterHistory {
    .currentRouteTime = 0.f,
    .history = {},
    .params = {},
	};
}

void pushHistory(RouterHistory& history, std::vector<std::string> newPath, bool replace, bool removeParams){
  history.currentRouteTime = gameapi -> timeSeconds(true);
  if (replace){
    history.history = {};
  }
  for (auto &path : newPath){
    history.history.push_back(path);
  }
  if (registerOnRouteChangedFn.has_value()){
    registerOnRouteChangedFn.value()();
  }
  if (removeParams){
    history.params = {};
  }
}

void popHistory(RouterHistory& history){
  if (history.history.size() == 0){
    return;
  }
  history.history.pop_back();
  history.currentRouteTime = gameapi -> timeSeconds(true);
  if (registerOnRouteChangedFn.has_value()){
    registerOnRouteChangedFn.value()();
  }  
}

void pushHistoryParam(RouterHistory& history, std::string param){
  for (auto &paramVal : history.params){
    if (paramVal ==  param){
      return;
    }
  }
  history.params.push_back(param);
  if (registerOnRouteChangedFn.has_value()){
    registerOnRouteChangedFn.value()();
  }  
}

void rmHistoryParam(RouterHistory& history, std::string param){
  auto originalSize = history.params.size();
  std::vector<std::string> newParams;
  for (auto &paramVal : history.params){
    if (paramVal != param){
      newParams.push_back(paramVal);
    }
  }
  history.params = newParams;
  if (history.params.size() != originalSize){
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

std::string fullDebugStr(RouterHistory& history){
  std::string str = "";
  for (auto &path : history.history){
    str += path + "/";
  }
  str += "#";
  for (auto &param : history.params){
    str += param + "/";
  }
  return str; 
}

std::string getCurrentPath(RouterHistory& history){
  return history.history.at(history.history.size() - 1);
}

std::vector<std::string> historyParams(RouterHistory& history){
  return history.params;
}

std::optional<std::string> getPathParts(RouterHistory& history, int index){
  if (history.history.size() <= index){
    return std::nullopt;
  }
  return history.history.at(index);
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


// can insert * instead of the subpath and that will match anything
PathMatch matchPath(std::string path, std::string expression){
  auto pathSplit = split(path, '/');
  auto expressionSplit = split(expression, '/');

  PathMatch noMatch {
    .matches = false,
    .params = {},
  };

  if (pathSplit.size() != expressionSplit.size()){
    return noMatch;
  }

  std::vector<std::string> params;
  for (int i = 0; i < pathSplit.size(); i++){
    if (expressionSplit.at(i) != pathSplit.at(i)){
      if (expressionSplit.at(i) == "*"){
        params.push_back(pathSplit.at(i));
      }else{
        return noMatch;
      }
    }
  }

  return PathMatch {
    .matches = true,
    .params = params,
  };
}

const Component* componentAtRoute(const std::map<std::string, Component>& routeToComponent, std::string& path){
  for (auto &[routePath, component] : routeToComponent){
    if (matchPath(path, routePath).matches){
      return &component;
    }
  }
  return NULL;
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
