#ifndef MOD_AFTERWORLD_COMPONENTS_ROUTER
#define MOD_AFTERWORLD_COMPONENTS_ROUTER

#include "./common.h"

struct RouterHistory {
  float currentRouteTime;
  std::deque<std::string> history;
  std::optional<std::any> data;
  std::optional<std::function<void(bool)>> registerOnRouteChangedFn;
};
RouterHistory createHistory();
void pushHistory(RouterHistory& history, std::vector<std::string> path, bool replace, std::optional<std::any> data = std::optional<std::any>(std::nullopt), bool forceLoad = false);
void popHistory(RouterHistory& history);
std::optional<std::any>& getData(RouterHistory& history);

std::string fullHistoryStr(RouterHistory& history);
std::string fullDebugStr(RouterHistory& history);
std::string getCurrentPath(RouterHistory& history);

std::optional<std::string> getPathParts(RouterHistory& history, int index);
Component withAnimator(RouterHistory& history, Component component, float duration);
void registerOnRouteChanged(RouterHistory& history, std::function<void(bool forceLoad)> onRouteChanged);


struct PathMatch {
  bool matches;
  std::vector<std::string> params;
};

// can insert * instead of the subpath and that will match anything
PathMatch matchPath(std::string path, std::string expression);


extern Component router;

#endif