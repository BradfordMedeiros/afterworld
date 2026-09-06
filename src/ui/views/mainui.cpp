#include "./mainui.h"

extern CustomApiBindings* gameapi;

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

Props createRouterProps(RouterHistory& routerHistory, std::optional<objid> selectedId){
  std::unordered_map<std::string, Component> routeToComponent = {
    { "mainmenu/levelselect/", withAnimator(routerHistory, withSimpleAnimatedLayout(emptyComponent), 0.125f) },
    { "mainmenu/settings/", withAnimator(routerHistory, withSimpleAnimatedLayout(emptyComponent), 0.25f) },
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

HandlerFns handleDrawMainUi(UiStateContext& uiStateContext, std::optional<objid> selectedId, std::optional<unsigned int> textureId, std::optional<glm::vec2> ndiCursor, bool editorMode){
  //////////////////////////////
  // navlist uses this via extern

  HandlerFns handlerFuncs {
    .minManagedId = -1,
    .maxManagedId = -1,
    .handlerFns = {},
    .handlerCallbackFns = {},
    .handlerFns2 = {},
    .inputFns = {},
    .trackedLocationIds = {},
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
  };
  resetMenuItemMappingId();

  if (!editorMode){
    auto routerProps = createRouterProps(*(uiStateContext.routerHistory), selectedId);
    router.draw(drawTools, routerProps);    

  }

  getMenuMappingData(&handlerFuncs.minManagedId, &handlerFuncs.maxManagedId);

  return handlerFuncs;
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

