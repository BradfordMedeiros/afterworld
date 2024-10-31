#ifndef MOD_AFTERWORLD_COMPONENTS_INDEX
#define MOD_AFTERWORLD_COMPONENTS_INDEX

#include "../components/router.h"
#include "./utilview.h"
#include "./pausemenu.h"
#include "./levelselect.h"
#include "./playing.h"
#include "./mainmenu.h"
#include "./settings.h"
#include "./modelviewer.h"
#include "./editorview.h"
#include "./windowmanager.h"
#include "./uicontext.h"
#include "../components/uiwindow.h"
#include "../components/game/elevator.h"
#include "../components/game/wheel.h"
#include "./navigation.h"
#include "../../modelviewer.h"

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
  int imageListScrollAmount;
  int fileexplorerScrollAmount;
  std::optional<std::function<void(bool closedWithoutNewFile, std::string file)>> onFileAddedFn;
  std::optional<std::function<void(objid, std::string)>> onGameObjSelected;
  std::optional<std::function<bool(bool isDirectory, std::string&)>> fileFilter;
  std::optional<std::function<void(bool closedWithoutInput, std::string input)>> onInputBoxFn;

  std::string colorPickerTitle;
  std::optional<std::function<void(glm::vec4)>> onNewColor;

  std::optional<objid> focusedId;
  std::string lastAutofocusedKey;

  bool showScenes;
  int offset;
  int currentScene;

  NavbarType navbarType;
  std::set<std::string> dockedDocks;

  
};
UiState createUiState();

struct UiStateContext {
  RouterHistory* routerHistory;
  UiState uiState;
};
HandlerFns handleDrawMainUi(UiStateContext& uiStateContext, UiContext& context, std::optional<objid> selectedId, std::optional<unsigned int> textureId, std::optional<glm::vec2> ndiCursor);
void onMainUiScroll(UiStateContext& uiStateContext,  UiContext& uiContext, double amount);
void onMainUiMousePress(UiStateContext& uiStateContext, UiContext& uiContext, HandlerFns& handlerFns, int button, int action, std::optional<objid> selectedId);
void onMainUiKeyPress(UiStateContext& uiStateContext, HandlerFns& handlerFns, int key, int scancode, int action, int mods);
void onMainUiMouseMove(UiStateContext& uiStateContext, UiContext& context, double xPos, double yPos, float xNdc, float yNdc);
void onMainUiObjectsChanged();
void pushHistory(std::vector<std::string> route, bool replace = false);
void pushHistoryParam(std::string param);
void rmHistoryParam(std::string param);


void popHistory();
std::string getCurrentPath();
std::vector<std::string> historyParams();
std::string fullHistoryStr();
std::optional<std::string> getPathParts(int index);
void sendUiAlert(std::string message);


#endif

