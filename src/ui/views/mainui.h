#ifndef MOD_AFTERWORLD_COMPONENTS_INDEX
#define MOD_AFTERWORLD_COMPONENTS_INDEX

#include "./playing.h"

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
std::string getCurrentPath(RouterHistory& history);

std::optional<std::string> getPathParts(RouterHistory& history, int index);
void registerOnRouteChanged(RouterHistory& history, std::function<void(bool forceLoad)> onRouteChanged);


struct PathMatch {
  bool matches;
  std::vector<std::string> params;
};

// can insert * instead of the subpath and that will match anything
PathMatch matchPath(std::string path, std::string expression);


RouterHistory& getMainRouterHistory();

struct UiStateContext {
  RouterHistory* routerHistory;
};
void pushHistory(std::vector<std::string> route, bool replace, std::optional<std::any> data = std::optional<std::any>(std::nullopt), bool forceLoad = false);
std::optional<std::any>& getData();

void popHistory();
std::string getCurrentPath();
std::string fullHistoryStr();
std::optional<std::string> getPathParts(int index);

#endif

