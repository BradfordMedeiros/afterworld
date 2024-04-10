#ifndef MOD_AFTERWORLD_COMPONENTS_ROUTER
#define MOD_AFTERWORLD_COMPONENTS_ROUTER

#include "./common.h"

struct RouterHistory {
  std::string currentPath;
  std::string initialRoute;
  float currentRouteTime;
  std::deque<std::string> history;
};
RouterHistory createHistory(std::string initialRoute);
void pushHistory(RouterHistory& history, std::string path, bool replace = false);
void popHistory(RouterHistory& history);
std::string fullHistoryStr(RouterHistory& history);
std::string getCurrentPath(RouterHistory& history);
Component withAnimator(RouterHistory& history, Component& component, float duration);
void registerOnRouteChanged(std::function<void()> onRouteChanged);

extern Component router;

#endif