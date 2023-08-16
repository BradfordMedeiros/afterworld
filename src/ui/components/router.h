#ifndef MOD_AFTERWORLD_COMPONENTS_ROUTER
#define MOD_AFTERWORLD_COMPONENTS_ROUTER

#include "./common.h"

struct RouterHistory {
  std::string currentPath;
};
RouterHistory createHistory(std::string initialRoute);
void pushHistory(RouterHistory& history, std::string path);
std::string getCurrentPath(RouterHistory& history);

extern Component router;

#endif