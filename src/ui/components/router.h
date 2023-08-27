#ifndef MOD_AFTERWORLD_COMPONENTS_ROUTER
#define MOD_AFTERWORLD_COMPONENTS_ROUTER

#include "./common.h"

struct RouterHistory {
  std::string currentPath;
  float currentRouteTime;
};
RouterHistory createHistory(std::string initialRoute);
void pushHistory(RouterHistory& history, std::string path);
std::string getCurrentPath(RouterHistory& history);
Component withAnimator(RouterHistory& history, Component& component, float duration);

extern Component router;

#endif