#include "./mainui.h"

extern CustomApiBindings* gameapi;


RouterHistory createHistory(){
  return RouterHistory {
    .currentRouteTime = 0.f,
    .history = {},
    .registerOnRouteChangedFn = std::nullopt,
  };
}

void pushHistory(RouterHistory& history, std::vector<std::string> newPath, bool replace, std::optional<std::any> data, bool forceLoad){
  history.currentRouteTime = gameapi -> timeSeconds(true);
  history.data = data;
  if (replace){
    history.history = {};
  }
  for (auto &path : newPath){
    history.history.push_back(path);
  }
  if (history.registerOnRouteChangedFn.has_value()){
    history.registerOnRouteChangedFn.value()(forceLoad);
  }
}

void popHistory(RouterHistory& history){
  if (history.history.size() == 0){
    return;
  }
  history.history.pop_back();
  history.currentRouteTime = gameapi -> timeSeconds(true);
  if (history.registerOnRouteChangedFn.has_value()){
    history.registerOnRouteChangedFn.value()(false);
  }  
}

std::optional<std::any>& getData(RouterHistory& history){
  return history.data;
}

std::string fullHistoryStr(RouterHistory& history){
  std::string str = "";
  for (auto &path : history.history){
    str += path + "/";
  }
  return str;
}


std::string getCurrentPath(RouterHistory& history){
  return history.history.at(history.history.size() - 1);
}

std::optional<std::string> getPathParts(RouterHistory& history, int index){
  if (history.history.size() <= index){
    return std::nullopt;
  }
  return history.history.at(index);
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

void registerOnRouteChanged(RouterHistory& history, std::function<void(bool)> onRouteChanged){
  modassert(!history.registerOnRouteChangedFn.has_value(), "can only register a single route");
  history.registerOnRouteChangedFn = onRouteChanged;
}


auto mainRouterHistory = createHistory();
RouterHistory& getMainRouterHistory(){
  return mainRouterHistory;
}

void pushHistory(std::vector<std::string> route, bool replace, std::optional<std::any> data, bool forceLoad){
  pushHistory(mainRouterHistory, route, replace, data, forceLoad);
}
void popHistory(){
  popHistory(mainRouterHistory);
}

std::optional<std::any>& getData(){
  return getData(mainRouterHistory);
}

std::string getCurrentPath(){
  return getCurrentPath(mainRouterHistory);
}

std::string fullHistoryStr(){
  return fullHistoryStr(mainRouterHistory);
}

std::optional<std::string> getPathParts(int index){
  return getPathParts(mainRouterHistory, index);
}

