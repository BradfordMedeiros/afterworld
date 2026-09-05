#ifndef MOD_AFTERWORLD_COMPONENTS_INDEX
#define MOD_AFTERWORLD_COMPONENTS_INDEX

#include "../components/router.h"
#include "../components/basic/layout.h"
#include "./playing.h"
#include "./navigation.h"

extern Component mainUI;

struct AutoFocusObj {
  objid id;
  std::string key;
};

struct HandlerFns {
  int minManagedId;
  int maxManagedId;
  std::unordered_map<objid, std::function<void()>> handlerFns;
  std::unordered_map<objid, std::function<void(HandlerCallbackFn&)>> handlerCallbackFns;    
  std::unordered_map<objid, std::function<void(int)>> handlerFns2;
  std::unordered_map<objid, std::function<void(int, int)>> inputFns;
  std::unordered_map<objid, TrackedLocationData> trackedLocationIds;
  std::optional<AutoFocusObj> autofocus;
};

RouterHistory& getMainRouterHistory();

struct UiState {
  std::optional<objid> focusedId;
  std::string lastAutofocusedKey;
};
UiState createUiState();

struct UiStateContext {
  RouterHistory* routerHistory;
  UiState uiState;
};
HandlerFns handleDrawMainUi(UiStateContext& uiStateContext, std::optional<objid> selectedId, std::optional<unsigned int> textureId, std::optional<glm::vec2> ndiCursor, bool editorMode);
void onMainUiMousePress(UiStateContext& uiStateContext, HandlerFns& handlerFns, int button, int action, std::optional<objid> selectedId);
void onMainUiKeyPress(UiStateContext& uiStateContext, HandlerFns& handlerFns, int key, int scancode, int action, int mods);
void pushHistory(std::vector<std::string> route, bool replace, std::optional<std::any> data = std::optional<std::any>(std::nullopt), bool forceLoad = false);
std::optional<std::any>& getData();

void popHistory();
std::string getCurrentPath();
std::string fullHistoryStr();
std::optional<std::string> getPathParts(int index);

#endif

