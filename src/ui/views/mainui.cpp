#include "./mainui.h"

extern CustomApiBindings* gameapi;

void setMenuBackground(std::string background);
void playRecordingBySignal(std::string signal, std::string rec, bool reverse);
bool isSignalLocked(std::string signal);

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
  auto pauseComponent = withPropsCopy(pauseMenuComponent, pauseMenuProps(selectedId, uiContext));
  auto deadComponent = withPropsCopy(pauseMenuComponent, deadMenuProps(selectedId, uiContext));

  auto levelSelect = withPropsCopy(
    levelSelectComponent, 
    Props { .props = {
        { .symbol = playLevelSymbol, .value = uiContext.consoleInterface.goToLevel } 
      }
    }
  );

  auto modelViewer = withPropsCopy(
    modelViewerComponent,
    Props {
      .props = {
        PropPair { .symbol = leftButtonSymbol, .value = uiContext.showPreviousModel },
        PropPair { .symbol = rightButtonSymbol, .value = uiContext.showNextModel },
      },
    }
  );

  auto particleViewer = withPropsCopy(
    particleViewerComponent,
    Props {
      .props = {
        PropPair { .symbol = leftButtonSymbol, .value = uiContext.showPreviousModel },
        PropPair { .symbol = rightButtonSymbol, .value = uiContext.showNextModel },
      },
    }
  );

  auto playingView = withPropsCopy(
    playingComponent,
    Props {
      .props = {
        PropPair {
          .symbol = valueSymbol, 
          .value = PlayingOptions { 
            .showHud = uiContext.showGameHud(),
            .showZoomOverlay = uiContext.showZoomOverlay(),
            .scoreOptions = uiContext.getScoreConfig(),
            .terminalConfig = uiContext.showTerminal(),
          } 
        },
      },
    }
  );

  auto elevatorView = withPropsCopy(
    elevatorComponent,
    Props {
      .props = {
        PropPair {
          .symbol = valueSymbol, 
          .value = ElevatorUiOptions {
            .onClickUp = []() -> void {
              modlog("main ui game", "elevator up");
              playRecordingBySignal("test", "../afterworld/data/recordings/move.rec", false);
            },
            .onClickDown = []() -> void {
              modlog("main ui game", "elevator down");
              playRecordingBySignal("test", "../afterworld/data/recordings/move.rec", true);
            },
            .canClickUp = !isSignalLocked("test"),
            .canClickDown = !isSignalLocked("test"),
          },
        },
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



  std::map<std::string, Component> routeToComponent = {
    { "mainmenu/",  mainMenu },
    { "mainmenu/levelselect/", withNavigation(uiContext, withAnimator(routerHistory, withSimpleAnimatedLayout(levelSelect), 0.125f)) },
    { "mainmenu/settings/", withNavigation(uiContext, withAnimator(routerHistory, withSimpleAnimatedLayout(settingsComponent), 0.25f)) },
    { "playing/*/",  playingView },
    { "playing/*/paused/", pauseComponent },
    { "playing/*/dead/", deadComponent },
    { "mainmenu/modelviewer/", withNavigation(uiContext, modelViewer) },
    { "mainmenu/particleviewer/", withNavigation(uiContext, particleViewer) },
    { "gamemenu/elevatorcontrol/", elevatorView },
    { "debug/wheel/",  simpleLayout(wheelView, glm::vec2(1.5f, 1.5f), defaultAlignment, glm::vec4(1.f, 0.f, 0.f, 1.f)) },
    { "loading/",  loadingComponent },
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
    .imageListScrollAmount = 0,
    .fileexplorerScrollAmount = 0,
    .onFileAddedFn = std::nullopt,
    .onGameObjSelected = std::nullopt,
    .fileFilter = std::nullopt,
    .onInputBoxFn = std::nullopt,
  
    .colorPickerTitle = "color picker",
    .onNewColor = std::nullopt,
  
    .focusedId = std::nullopt,
    .lastAutofocusedKey = "",
  
    .showScenes = false,
    .offset = 2,
    .currentScene = -1,
  
    .navbarType = MAIN_EDITOR,
    .dockedDocks = {},
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
  .uiMainContext = UiMainContext {
    .openNewSceneMenu = [](std::function<void(bool closedWithoutInput, std::string input)> onInputBox) -> void { // replace with onInputBoxFn
      modassert(commonState, "commonState is NULL");
      windowSetEnabled(windowDialogSymbol, true);
      commonState -> onInputBoxFn = [onInputBox](bool closedWithoutNewFile, std::string userInput) -> void {
        onInputBox(closedWithoutNewFile, userInput);
        std::cout << "open new scene user input is: " << userInput << std::endl;;
        commonState -> onInputBoxFn = std::nullopt;
        windowSetEnabled(windowDialogSymbol, false);
      };
      std::cout << "open new scene placeholder" << std::endl;
    }
  }
};


DockConfigApi dockConfigApi { // probably should be done via a prop for better control flow
  .createCamera = []() -> void {
    std::map<std::string, GameobjAttributes> submodelAttributes;
    GameobjAttributes attr { .attr = {} };
    gameapi -> makeObjectAttr(uiManagerContext.uiContext -> activeSceneId().value(), std::string(">camera-") + uniqueNameSuffix(), attr, submodelAttributes);
  },
  .createLight = []() -> void {
    std::map<std::string, GameobjAttributes> submodelAttributes;
    GameobjAttributes attr { .attr = {} };
    gameapi -> makeObjectAttr(uiManagerContext.uiContext -> activeSceneId().value(), std::string("!light-") + uniqueNameSuffix(), attr, submodelAttributes);
  },
  .createNavmesh = []() -> void {
    std::map<std::string, GameobjAttributes> submodelAttributes;
    GameobjAttributes attr { .attr = {} };
    gameapi -> makeObjectAttr(uiManagerContext.uiContext -> activeSceneId().value(), std::string(";navmesh-") + uniqueNameSuffix(), attr, submodelAttributes);
  },
  .openFilePicker = [](std::function<void(bool closedWithoutNewFile, std::string file)> onFileAdded, std::function<bool(bool, std::string&)> fileFilterFn) -> void {
    windowSetEnabled(windowFileExplorerSymbol, true);
    commonState -> onFileAddedFn = [onFileAdded](bool closedWithoutNewFile, std::string file) -> void {
      onFileAdded(closedWithoutNewFile, file);
      commonState -> onFileAddedFn = std::nullopt;
      commonState -> fileFilter = std::nullopt;
      windowSetEnabled(windowFileExplorerSymbol, false);
    };
    commonState -> fileFilter = fileFilterFn;
  },
  .openImagePicker = [](std::function<void(bool closedWithoutNewFile, std::string file)> onFileAdded) -> void {
    windowSetEnabled(windowImageExplorerSymbol, true);
    commonState -> onFileAddedFn = [onFileAdded](bool closedWithoutNewFile, std::string file) -> void {
      onFileAdded(closedWithoutNewFile, file);
      commonState -> onFileAddedFn = std::nullopt;
      windowSetEnabled(windowImageExplorerSymbol, false);
    };
  },
  .openColorPicker = [](std::function<void(glm::vec4)> onColor, std::string windowName) -> void {
    windowSetEnabled(windowColorPickerSymbol, true);
    commonState -> onNewColor = onColor;
    commonState -> colorPickerTitle = windowName;
    //onFileAddedFn = [onFileAdded](bool closedWithoutNewFile, std::string file) -> void {
    //  onFileAdded(closedWithoutNewFile, file);
    //  onFileAddedFn = std::nullopt;
    //  fileFilter = std::nullopt;
    //  windowSetEnabled(windowFileExplorerSymbol, false);
    //};
    //fileFilter = fileFilterFn;
  },
  .pickGameObj = [](std::function<void(objid, std::string)> selectGameObj) -> void {
    std::cout << "dock pick gameobj" << std::endl;
    // kind of hack, otherwise it would select the object just clicked due to ordering of binding callbacks
    gameapi -> schedule(-1, 100, NULL, [selectGameObj](void*) -> void { 
      commonState -> onGameObjSelected = selectGameObj;
    });
  },
  .setTexture = [](std::string& texture) -> void {
    std::cout << "dock mock set texture: " << texture << std::endl;
    auto selectedIds = gameapi -> selected();
    for (auto id : selectedIds){
      setGameObjectTexture(id, texture);     
    }
  },
  .getAttribute = [](std::string key, std::string attribute) -> AttributeValue {
    auto value = getWorldState(key.c_str(), attribute.c_str()).value();
    return value;
  },
  .setAttribute = [](std::string key, std::string attribute, AttributeValue value) -> void {
    gameapi -> setWorldState({
      ObjectValue {
        .object = key,
        .attribute = attribute,
        .value = value,
      },
    });
  },
  .getObjAttr = [](std::string key) -> std::optional<AttributeValue> {
    auto selected = gameapi -> selected();
    if (selected.size() == 0){
      return std::nullopt;
    }
    objid id = selected.at(0);
    auto gameobjAttr = getAttrHandle(id);
    auto attr = getAttr(gameobjAttr, key.c_str());
    return attr;
  },
  .setObjAttr = [](std::string key, AttributeValue value) -> void {
    auto selected = gameapi -> selected();
    if (selected.size() == 0){
      return;
    }
    objid id = selected.at(0);
    gameapi -> setSingleGameObjectAttr(id, key.c_str(), value);
  },
  .setEditorBackground = setMenuBackground,
  .emitParticleViewerParticle = emitNewParticleViewerParticle,
  .setParticlesViewerShouldEmit = setParticlesViewerShouldEmit,
  .getParticlesViewerShouldEmit = getParticlesViewerShouldEmit,
  .setParticleAttribute = setParticleAttribute,
  .getParticleAttribute = getParticleAttribute,
};


NavbarType queryLoadNavbarType(){
  auto query = gameapi -> compileSqlQuery("select layout from session", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  return strToNavbarType(result.at(0).at(0));
}
void queryUpdateNavbarType(std::string& layout){
  auto updateQuery = gameapi -> compileSqlQuery(
    "update session set layout = ?", { layout }
  );
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(updateQuery, &validSql);
  modassert(validSql, "error executing sql query");
}

NavListApi navListApi {
  .changeLayout = [](std::string layout) -> void {
    commonState -> navbarType = strToNavbarType(layout);
    queryUpdateNavbarType(layout);
  }
};

ImageList loadImageListTextures(){
  ImageList imageListDatas {
    .images = {},
  };
  auto allTextures = gameapi -> listResources("textures");;
  imageListDatas.images = {};
  for (auto &texture : allTextures){
    imageListDatas.images.push_back(ImageListImage {
      .image = texture,
    });
  }
  modlog("mainui", "loadImageListTextures");
  return imageListDatas;
}


static bool firstTime = true;
HandlerFns handleDrawMainUi(UiStateContext& uiStateContext, UiContext& uiContext, std::optional<objid> selectedId, std::optional<unsigned int> textureId, std::optional<glm::vec2> ndiCursor){
  UiState& uiState = uiStateContext.uiState;
  commonState = &uiState;

  if (firstTime){
    initStyles();
    uiState.navbarType = queryLoadNavbarType();
  }
  firstTime = false;
  static ImageList imageListDatas = loadImageListTextures();

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

  auto routerProps = createRouterProps(*(uiStateContext.routerHistory), uiContext, selectedId);
  router.draw(drawTools, routerProps);

  {
    Props defaultProps {
      .props = {
        PropPair {
          .symbol = valueSymbol, 
          .value = UtilViewOptions {
            .showKeyboard = uiContext.showKeyboard(),
            .showConsole = uiContext.showConsole(),
            .showScreenspaceGrid = uiContext.showScreenspaceGrid(),
            .consoleKeyName = (std::string("console-") + uniqueNameSuffix()),
            .consoleInterface = &uiContext.consoleInterface,
            .ndiCursor = ndiCursor,
            .debugConfig = uiContext.debugConfig(),
          } 
        },
      },
    };
    utilViewComponent.draw(drawTools, defaultProps);
  }

  if (uiContext.showEditor()){
    auto onClickNavbar = [&uiState](const char* value) -> void {
      uiState.dockedDocks.insert(value);
      for (auto &dock : uiState.dockedDocks){
        auto windowDockSymbol = getSymbol(std::string("window-symbol-") + dock);
        windowSetEnabled(windowDockSymbol, true, glm::vec2(1.f, 0.9f));    
      }
    };

    Props editorViewProps {
      .props = {
        PropPair {
          .symbol = valueSymbol, 
          .value = EditorViewOptions { 
            .worldPlayInterface = &uiContext.worldPlayInterface,
            .onNewColor = uiState.onNewColor,
            .colorPickerTitle = &uiState.colorPickerTitle,
            .navbarType = uiState.navbarType,
            .onClickNavbar = onClickNavbar,
            .onFileAddedFn = uiState.onFileAddedFn,
            .fileexplorerScrollAmount = uiState.fileexplorerScrollAmount,
            .fileFilter = uiState.fileFilter,
            .onInputBoxFn = uiState.onInputBoxFn,
            .imageListDatas = &imageListDatas,
            .imageListScrollAmount = uiState.imageListScrollAmount,
            .dockedDocks = &uiState.dockedDocks,
            .sceneManagerInterface = SceneManagerInterface {
              .showScenes = uiState.showScenes,
              .offset = uiState.offset,
              .onSelectScene = [&uiState, &uiContext](int index, std::string scene) -> void {
                uiContext.loadScene(scene);
                uiState.currentScene = index;
                uiState.showScenes = false;
              },
              .toggleShowScenes = [&uiState]() -> void {
                uiState.showScenes = !uiState.showScenes;
              },
              .scenes = uiContext.listScenes(),
              .currentScene = uiState.currentScene,
            },
          } 
        },
      }
    };
    editorViewComponent.draw(drawTools, editorViewProps);
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
  uiState.imageListScrollAmount += (scrollValue * 5);
  if (uiState.imageListScrollAmount < 0){
    uiState.imageListScrollAmount = 0;
  }

  uiState.fileexplorerScrollAmount += scrollValue;
  if (uiState.fileexplorerScrollAmount < 0){
    uiState.fileexplorerScrollAmount = 0;
  }

  uiState.offset += scrollValue;
  if (uiState.offset < 0){
    uiState.offset = 0;
  }
  scenegraphScroll(scrollValue);


  rotateWheel(amount > 0);
}

void onMainUiMousePress(UiStateContext& uiStateContext, UiContext& uiContext, HandlerFns& handlerFns, int button, int action, std::optional<objid> selectedId){
  modassert(handlerFns.minManagedId, "handlerfns minManagedId invalid data");
  modassert(handlerFns.maxManagedId, "handlerfns maxManagedId invalid data");

  UiState& uiState = uiStateContext.uiState;
  commonState = &uiState;

  std::cout << "button: " << button << ", action: " << action << std::endl;
  if (button == 0 && action == 0){
    windowOnRelease();
    std::cout << uiStoreToStr() << std::endl;
  }
  if (button == 0 && action == 1){
    if (selectedId.has_value() && uiState.onGameObjSelected.has_value()){
      auto gameobjName = gameapi -> getGameObjNameForId(selectedId.value()).value();
      uiState.onGameObjSelected.value()(selectedId.value(), gameobjName);
      uiState.onGameObjSelected = std::nullopt;
    }

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
  refreshScenegraph();
}

auto mainRouterHistory = createHistory();
RouterHistory& getMainRouterHistory(){
  return mainRouterHistory;
}

void pushHistory(std::vector<std::string> route, bool replace){
  pushHistory(mainRouterHistory, route, replace);
}
void popHistory(){
  popHistory(mainRouterHistory);
}

void pushHistoryParam(std::string param){
  pushHistoryParam(mainRouterHistory, param);
}
void rmHistoryParam(std::string param){
  rmHistoryParam(mainRouterHistory, param);
}

std::string getCurrentPath(){
  return getCurrentPath(mainRouterHistory);
}

std::vector<std::string> historyParams(){
  return historyParams(mainRouterHistory);
}

std::string fullHistoryStr(){
  return fullHistoryStr(mainRouterHistory);
}

std::optional<std::string> getPathParts(int index){
  return getPathParts(mainRouterHistory, index);
}

void sendUiAlert(std::string message){
  modlog("dock alert", message);
  pushAlertMessage(message, ALERT_DETAIL);
}