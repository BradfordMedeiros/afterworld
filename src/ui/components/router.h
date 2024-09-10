#ifndef MOD_AFTERWORLD_COMPONENTS_ROUTER
#define MOD_AFTERWORLD_COMPONENTS_ROUTER

#include "./common.h"

struct RouterHistory {
  float currentRouteTime;
  std::deque<std::string> history;
  std::vector<std::string> params;
  std::optional<std::function<void()>> registerOnRouteChangedFn;
};
RouterHistory createHistory();
void pushHistory(RouterHistory& history, std::vector<std::string> path, bool replace = false, bool removeParams = false);
void popHistory(RouterHistory& history);

void pushHistoryParam(RouterHistory& history, std::string param);
void rmHistoryParam(RouterHistory& history, std::string param);

std::string fullHistoryStr(RouterHistory& history);
std::string fullDebugStr(RouterHistory& history);
std::string getCurrentPath(RouterHistory& history);
std::vector<std::string> historyParams(RouterHistory& history);

std::optional<std::string> getPathParts(RouterHistory& history, int index);
Component withAnimator(RouterHistory& history, Component& component, float duration);
void registerOnRouteChanged(RouterHistory& history, std::function<void()> onRouteChanged);


struct PathMatch {
  bool matches;
  std::vector<std::string> params;
};

// can insert * instead of the subpath and that will match anything
PathMatch matchPath(std::string path, std::string expression);


extern Component router;

#endif