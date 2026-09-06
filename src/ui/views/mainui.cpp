#include "./mainui.h"


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

