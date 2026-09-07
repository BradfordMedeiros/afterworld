#include "./mainui.h"

extern CustomApiBindings* gameapi;
extern UiMode uiMode;
extern RouterHistory mainRouterHistory;

std::optional<TerminalConfig> terminal;


RouterHistory createHistory(){
  return RouterHistory {
    .currentRouteTime = 0.f,
    .history = {},
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
  history.routeChangedForceReload = forceLoad;
}

void popHistory(RouterHistory& history){
  if (history.history.size() == 0){
    return;
  }
  history.history.pop_back();
  history.currentRouteTime = gameapi -> timeSeconds(true);
  history.routeChangedForceReload = false;
}

std::string fullHistoryStr(RouterHistory& history){
  std::string str = "";
  for (auto &path : history.history){
    str += path + "/";
  }
  return str;
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

void pushHistory(std::vector<std::string> route, bool replace, std::optional<std::any> data, bool forceLoad){
  pushHistory(mainRouterHistory, route, replace, data, forceLoad);
}
void popHistory(){
  popHistory(mainRouterHistory);
}

std::optional<BallModeUi*> getBallModeUI(){
  auto uiModeBall = std::get_if<BallModeUi>(&uiMode);
  if (uiModeBall == NULL){
    return std::nullopt;
  }
  return uiModeBall;
}

std::optional<LiveMenu*> getLiveMenuUi(){
  auto liveMenu = std::get_if<LiveMenu>(&uiMode);
  if (liveMenu == NULL){
    return std::nullopt;
  }
  return liveMenu;
}

void setTerminalConfig(std::optional<TerminalConfig> terminalConfig){
  terminal = terminalConfig;
}
std::optional<TerminalConfig*> getTerminalConfig(){
  if (!terminal.has_value()){
    return std::nullopt;
  }  
  return &terminal.value();
}

