#include "./mainui.h"

extern CustomApiBindings* gameapi;

void setMenuBackground(std::string background);
objid createPrefab(objid sceneId, const char* prefab, glm::vec3 pos, std::unordered_map<std::string, AttributeValue> additionalFields);

float wheelRotationOffset = 0.f;
float actualWheelRotationOffset = wheelRotationOffset;
int wheelRotate = 0;
void rotateWheel(bool up);
WheelConfig wheelConfig {
  .numElementsInWheel = 10,
  .wheelRadius = 0.5f,
  .selectedIndex = 0,
  .getWheelContent = [](int index) -> std::optional<std::string> {
    if ((index - wheelRotate) == 0){
      return std::nullopt;
    }
    return std::string("placeholder content: " + std::to_string(index));
  },
  .getRotationOffset = []() -> float {
    actualWheelRotationOffset = glm::lerp(actualWheelRotationOffset, wheelRotationOffset, static_cast<float>(gameapi -> timeElapsed()));
    return actualWheelRotationOffset;
  },
  .onClick = [](int index) -> void {
    wheelRotate = index - 1;
    rotateWheel(true);
  },
};

void rotateWheel(bool up){
  wheelRotate += (up ? 1 : -1);
  if (wheelRotate < 0){
    wheelRotate = 0;
  }
  auto wheelDegreesPerRotate = (2 * M_PI) / wheelConfig.numElementsInWheel;
  wheelRotationOffset = -1 * wheelRotate * wheelDegreesPerRotate;
  wheelConfig.selectedIndex = wheelRotate;
}

Component withSimpleAnimatedLayout(Component& component){
  Component simpleAnimatedLayout {
    .draw = [component](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
      float* interpolationAmount = typeFromProps<float>(props, interpolationSymbol);
      modassert(interpolationAmount, "interpolationAmount undefined");  

      float xoffset = -2.f;
      if (interpolationAmount){
        xoffset += (*interpolationAmount * 2.f);
      }

      float opacity = *interpolationAmount - 0.1f;
      if (opacity < 0){
        opacity = 0.f;
      }
      drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(0.2f, 0.2f, 0.2f, opacity), true, std::nullopt, "../gameresources/build/textures/evilpattern.png", std::nullopt, std::nullopt);

      Layout layout {
        .tint = glm::vec4(0.f, 0.f, 0.f, 1.f - *interpolationAmount),
        .showBackpanel = true,
        .borderColor = glm::vec4(1.f, 0.f, 0.f, 0.f),
        .minwidth = 0.f,
        .minheight = 0.f,
        .layoutType = LAYOUT_HORIZONTAL2,
        .layoutFlowHorizontal = UILayoutFlowNone2,
        .layoutFlowVertical = UILayoutFlowNone2,
        .alignHorizontal = UILayoutFlowNone2,
        .alignVertical = UILayoutFlowNone2,
        .spacing = 0.f,
        .minspacing = 0.f,
        .padding = 0.f,
        .children = { component },
      };
      Props listLayoutProps {
        .props = {
          { .symbol = layoutSymbol, .value = layout },
          { .symbol = xoffsetSymbol, xoffset },
        },
      };
      return layoutComponent.draw(drawTools, listLayoutProps);
    }
  };
  return simpleAnimatedLayout;
}

Props createRouterProps(RouterHistory& routerHistory, UiContext& uiContext, std::optional<objid> selectedId){
  auto playingView = withPropsCopy(
    playingComponent,
    Props {
      .props = {
      },
    }
  );

  auto wheelView = withPropsCopy(
    wheelComponent,
    Props {
      .props = {
        PropPair {
          .symbol = valueSymbol, 
          .value = wheelConfig,
        },
      },
    }
  );



  std::unordered_map<std::string, Component> routeToComponent = {
    { "mainmenu/",  emptyComponent },
    { "mainmenu/levelselect/", withNavigation(uiContext, withAnimator(routerHistory, withSimpleAnimatedLayout(emptyComponent), 0.125f)) },
    { "mainmenu/settings/", withNavigation(uiContext, withAnimator(routerHistory, withSimpleAnimatedLayout(emptyComponent), 0.25f)) },
    { "playing/*/",  playingView },
    { "debug/wheel/",  simpleLayout(wheelView, glm::vec2(1.5f, 1.5f), defaultAlignment, glm::vec4(1.f, 0.f, 0.f, 1.f)) },
    { "",  emptyComponent  },
  };

  Props routerProps {
    .props = {
      { routerSymbol, routerHistory },
      { routerMappingSymbol, routeToComponent },
    },
  };
  return routerProps;
}

UiState createUiState(){
  UiState uiState {    
    .focusedId = std::nullopt,
    .lastAutofocusedKey = "",
  };
  return uiState;
}
UiState* commonState = NULL;

std::optional<AttributeValue> getWorldState(const char* object, const char* attribute){
  auto worldStates = gameapi -> getWorldState();
  for (auto &worldState : worldStates){
    if (worldState.object == object && worldState.attribute == attribute){
      return worldState.value;
    }
  }
  return std::nullopt;
}

UiManagerContext uiManagerContext {
  .uiContext = NULL,
};


static bool firstTime = true;
HandlerFns handleDrawMainUi(UiStateContext& uiStateContext, UiContext& uiContext, std::optional<objid> selectedId, std::optional<unsigned int> textureId, std::optional<glm::vec2> ndiCursor, bool editorMode){
  UiState& uiState = uiStateContext.uiState;
  commonState = &uiState;

  if (firstTime){
    initStyles();
  }
  firstTime = false;

  //////////////////////////////
  // navlist uses this via extern

  uiManagerContext.uiContext = &uiContext;

  HandlerFns handlerFuncs {
    .minManagedId = -1,
    .maxManagedId = -1,
    .handlerFns = {},
    .handlerCallbackFns = {},
    .handlerFns2 = {},
    .inputFns = {},
    .trackedLocationIds = {},
    .autofocus = std::nullopt,
  };
  //std::cout << "focusedId: " << (focusedId.has_value() ? std::to_string(focusedId.value()) : "no value") << std::endl;

  DrawingTools drawTools {
     .drawText = [&textureId](std::string word, float left, float top, unsigned int fontSize, bool permatext, std::optional<glm::vec4> tint, std::optional<unsigned int>, bool ndi, std::optional<std::string> fontFamily, std::optional<objid> selectionId, std::optional<float> maxWidth, std::optional<ShapeOptions> shaderId) -> void {
        gameapi -> drawText(word, left, top, fontSize, permatext, tint, textureId, ndi, fontFamily, selectionId, maxWidth, shaderId);
     },
     .getTextDimensionsNdi = gameapi -> getTextDimensionsNdi,
     .drawRect = [&handlerFuncs, &textureId](float centerX, float centerY, float width, float height, bool perma, std::optional<glm::vec4> tint, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture, std::optional<ShapeOptions> shaderId, std::optional<objid> trackingId) -> void {
      if (trackingId.has_value()){
        handlerFuncs.trackedLocationIds[trackingId.value()] = TrackedLocationData {
          .position = glm::vec2(centerX, centerY),
          .size = glm::vec2(width, height),
        };
      }
      gameapi -> drawRect(centerX, centerY, width, height, perma, tint, textureId, ndi, selectionId, texture, shaderId);
     },
     .drawLine2D = [&textureId](glm::vec3 fromPos, glm::vec3 toPos, bool perma, std::optional<glm::vec4> tint, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture, std::optional<ShapeOptions> shaderId) -> void {
        gameapi -> drawLine2D(fromPos, toPos, perma, tint, textureId, ndi, selectionId, texture, shaderId);
     },
     .registerCallbackFns = [&handlerFuncs](objid id, std::function<void()> fn) -> void {
        handlerFuncs.handlerFns[id] = fn;
     },
     .registerCallbackFnsHandler = [&handlerFuncs](objid id, std::function<void(HandlerCallbackFn&)> fn) -> void {
        handlerFuncs.handlerCallbackFns[id] = fn;
     },
     .registerCallbackRightFns = [&handlerFuncs](objid id, std::function<void(int)> fn) -> void {
        handlerFuncs.handlerFns2[id] = fn;
     },
     .registerInputFns = [&handlerFuncs](objid id, std::function<void(int, int)> fn) -> void {
        handlerFuncs.inputFns[id] = fn;
     },
     .registerAutoFocus = [&handlerFuncs](objid id, std::string& key) -> void {
        handlerFuncs.autofocus = AutoFocusObj {
          .id = id,
          .key = key,
        };
     },
     .selectedId = selectedId,
     .focusedId = uiState.focusedId,
     .getClipboardString = gameapi -> getClipboardString,
     .setClipboardString = gameapi -> setClipboardString,
  };
  resetMenuItemMappingId();

  if (!editorMode){
    auto routerProps = createRouterProps(*(uiStateContext.routerHistory), uiContext, selectedId);
    router.draw(drawTools, routerProps);    

  }


  {
    Props defaultProps {
      .props = {
        PropPair {
          .symbol = valueSymbol, 
          .value = UtilViewOptions {
            .showKeyboard = uiContext.showKeyboard(),
            .showScreenspaceGrid = uiContext.showScreenspaceGrid(),
            .consoleKeyName = (std::string("console-") + uniqueNameSuffix()),
            .ndiCursor = ndiCursor,
            .debugConfig = uiContext.debugConfig(),
          } 
        },
      },
    };
    utilViewComponent.draw(drawTools, defaultProps);
  }

  {
    Props props {
      .props = {},
    };
    fadeComponent.draw(drawTools, props);
  }

  getMenuMappingData(&handlerFuncs.minManagedId, &handlerFuncs.maxManagedId);

  if (handlerFuncs.autofocus.has_value()){
    if (uiState.lastAutofocusedKey != handlerFuncs.autofocus.value().key){
      uiState.focusedId = handlerFuncs.autofocus.value().id;
      uiState.lastAutofocusedKey = handlerFuncs.autofocus.value().key;
    }
  }

  if (uiContext.isDebugMode()){
    drawTools.drawText(std::string("route: ") + fullDebugStr(*(uiStateContext.routerHistory)), -0.8f, -0.95f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
    drawTools.drawText(std::string("handlers: ") + std::to_string(handlerFuncs.handlerFns.size()), -0.8f, -0.90f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
    drawTools.drawText(std::string("inputfns: ") + std::to_string(handlerFuncs.inputFns.size()), -0.8f, -0.85f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  }

  return handlerFuncs;
}

void onMainUiScroll(UiStateContext& uiStateContext,  UiContext& uiContext, double amount){
  UiState& uiState = uiStateContext.uiState;
  commonState = &uiState;

  auto scrollValue = static_cast<int>(amount);
  std::cout << "dock: on main ui scroll: " << scrollValue << std::endl;

  rotateWheel(amount > 0);
}

void onMainUiMousePress(UiStateContext& uiStateContext, UiContext& uiContext, HandlerFns& handlerFns, int button, int action, std::optional<objid> selectedId){
  //modassert(handlerFns.minManagedId, "handlerfns minManagedId invalid data");
  //modassert(handlerFns.maxManagedId, "handlerfns maxManagedId invalid data");

  UiState& uiState = uiStateContext.uiState;
  commonState = &uiState;

  std::cout << "button: " << button << ", action: " << action << std::endl;
  if (button == 0 && action == 1){
    if (selectedId.has_value() &&  selectedId.value() >= handlerFns.minManagedId &&  selectedId.value() <= handlerFns.maxManagedId){
      uiState.focusedId = selectedId.value();
    }

    if (selectedId.has_value()){
      if (handlerFns.handlerFns.find(selectedId.value()) != handlerFns.handlerFns.end()){
        handlerFns.handlerFns.at(selectedId.value())();
        uiContext.playSound();
      }
      if (handlerFns.handlerCallbackFns.find(selectedId.value()) != handlerFns.handlerCallbackFns.end()){\
        HandlerCallbackFn data{
          .trackedLocationData = handlerFns.trackedLocationIds.at(selectedId.value()),
        };
        handlerFns.handlerCallbackFns.at(selectedId.value())(data);
      }
    }
  }

  if (action == 1){
    if (selectedId.has_value()){
      if (handlerFns.handlerFns2.find(selectedId.value()) != handlerFns.handlerFns2.end()){
        handlerFns.handlerFns2.at(selectedId.value())(button);
      }
    }  
  }

}

void onMainUiKeyPress(UiStateContext& uiStateContext, HandlerFns& handlerFns, int key, int scancode, int action, int mods){
  UiState& uiState = uiStateContext.uiState;
  commonState = &uiState;

  modlog("mainui key press", std::to_string(key));
  modlog("mainui key press focused", print(uiState.focusedId));
  if (!uiState.focusedId.has_value()){
    return;
  }
  if (handlerFns.inputFns.find(uiState.focusedId.value()) != handlerFns.inputFns.end()){
    handlerFns.inputFns.at(uiState.focusedId.value())(key, mods);
  }
}

void onMainUiMouseMove(UiStateContext& uiStateContext, UiContext& context, double xPos, double yPos, float xNdc, float yNdc){
  
}

void onMainUiObjectsChanged(){
}

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

