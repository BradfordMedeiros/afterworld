#ifndef MOD_AFTERWORLD_COMPONENTS_INDEX
#define MOD_AFTERWORLD_COMPONENTS_INDEX

#include "../components/router.h"
#include "./playing.h"

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

