#include "./mainui.h"

extern CustomApiBindings* gameapi;

void setMenuBackground(std::string background);

auto routerHistory = createHistory();

Props createRouterProps(UiContext& uiContext, std::optional<objid> selectedId){
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

  std::map<std::string, Component> routeToComponent = {
    { "mainmenu/",  mainMenu },
    { "mainmenu/levelselect/", withNavigation(uiContext, levelSelect) },
    { "mainmenu/settings/", withNavigation(uiContext, settingsComponent) },
    { "playing/*/",  playingView },
    { "playing/*/paused/", pauseComponent },
    { "playing/*/dead/", deadComponent },
    { "mainmenu/modelviewer/", withNavigation(uiContext, modelViewer) },
    { "mainmenu/particleviewer/", withNavigation(uiContext, particleViewer) },
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

std::set<std::string> dockedDocks = {};
std::function<void(const char*)> onClickNavbar = [](const char* value) -> void {
  //pushQueryParam(routerHistory, "dockedDock");
  dockedDocks.insert(value);
  for (auto &dock : dockedDocks){
    auto windowDockSymbol = getSymbol(std::string("window-symbol-") + dock);
    windowSetEnabled(windowDockSymbol, true, glm::vec2(1.f, 0.9f));    
  }
};

std::optional<std::function<void(bool closedWithoutNewFile, std::string file)>> onFileAddedFn = std::nullopt;
std::optional<std::function<void(objid, std::string)>> onGameObjSelected = std::nullopt;
std::optional<std::function<bool(bool isDirectory, std::string&)>> fileFilter = std::nullopt;
std::optional<std::function<void(bool closedWithoutInput, std::string input)>> onInputBoxFn = std::nullopt;

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
      windowSetEnabled(windowDialogSymbol, true);
      onInputBoxFn = [onInputBox](bool closedWithoutNewFile, std::string userInput) -> void {
        onInputBox(closedWithoutNewFile, userInput);
        std::cout << "open new scene user input is: " << userInput << std::endl;;
        onInputBoxFn = std::nullopt;
        windowSetEnabled(windowDialogSymbol, false);
      };
      std::cout << "open new scene placeholder" << std::endl;
    }
  }
};

std::string colorPickerTitle = "color picker";
std::optional<std::function<void(glm::vec4)>> onNewColor = std::nullopt;


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
    onFileAddedFn = [onFileAdded](bool closedWithoutNewFile, std::string file) -> void {
      onFileAdded(closedWithoutNewFile, file);
      onFileAddedFn = std::nullopt;
      fileFilter = std::nullopt;
      windowSetEnabled(windowFileExplorerSymbol, false);
    };
    fileFilter = fileFilterFn;
  },
  .openImagePicker = [](std::function<void(bool closedWithoutNewFile, std::string file)> onFileAdded) -> void {
    windowSetEnabled(windowImageExplorerSymbol, true);
    onFileAddedFn = [onFileAdded](bool closedWithoutNewFile, std::string file) -> void {
      onFileAdded(closedWithoutNewFile, file);
      onFileAddedFn = std::nullopt;
      windowSetEnabled(windowImageExplorerSymbol, false);
    };
  },
  .openColorPicker = [](std::function<void(glm::vec4)> onColor, std::string windowName) -> void {
    windowSetEnabled(windowColorPickerSymbol, true);
    onNewColor = onColor;
    colorPickerTitle = windowName;
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
      onGameObjSelected = selectGameObj;
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

ImageList imageListDatas {
  .images = {},
};
int imageListScrollAmount = 0;
int fileexplorerScrollAmount = 0;

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

NavbarType navbarType = MAIN_EDITOR;
NavListApi navListApi {
  .changeLayout = [](std::string layout) -> void {
    navbarType = strToNavbarType(layout);
    queryUpdateNavbarType(layout);
  }
};


bool showScenes = false;
int offset = 2;
int currentScene = -1;

std::optional<objid> focusedId = std::nullopt;
std::string lastAutofocusedKey = "";

TextData newSceneTextData {
  .valueText = "",
  .cursorLocation = 0,
  .highlightLength = 0,
  .maxchars = -1,
};

static bool firstTime = true;
HandlerFns handleDrawMainUi(UiContext& uiContext, std::optional<objid> selectedId, std::optional<unsigned int> textureId, std::optional<glm::vec2> ndiCursor){
  if (firstTime){
    initStyles();
    navbarType = queryLoadNavbarType();
  }
  firstTime = false;
  //////////////////////////////

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
     .focusedId = focusedId,
     .getClipboardString = gameapi -> getClipboardString,
     .setClipboardString = gameapi -> setClipboardString,
  };
  resetMenuItemMappingId();

  for (auto &dockedDock : dockedDocks){
    Props dockProps { 
      .props = {
        { dockTypeSymbol, dockedDock }, 
        { xoffsetFromSymbol, 1.5f },
      }
    };
    auto dock = withProps(dockComponent, dockProps);
    auto defaultWindowProps = getDefaultProps();

    auto windowDockSymbol = getSymbol(std::string("window-symbol-") + dockedDock);
    std::function<void()> onClickX = [dockedDock]() -> void {
      dockedDocks.erase(dockedDock);
    };
    createUiWindow(
      dock, 
      windowDockSymbol, 
      onClickX, 
      dockedDock, 
      AlignmentParams { .layoutFlowHorizontal = UILayoutFlowNegative2, .layoutFlowVertical = UILayoutFlowNegative2 }
    ).draw(drawTools, defaultWindowProps);
  }

  {
    std::vector<ListComponentData> dialogOptions = {
      ListComponentData {
        .name = "confirm",
        .onClick = []() -> void {
          std::cout << "dialog confirm on click" << std::endl;
          if (onInputBoxFn.has_value()){
            onInputBoxFn.value()(false, newSceneTextData.valueText);
          }
        },      
      },
      ListComponentData {
        .name = "cancel",
        .onClick = []() -> void {
          if (onInputBoxFn.has_value()){
            onInputBoxFn.value()(true, "");
          }
        },      
      },
    };

    {
      std::function<void(TextData, int)> onEdit = [](TextData textData, int rawKey) -> void {
        newSceneTextData = textData;
      };
      Props dialogProps {
        .props = {
          PropPair { .symbol = listItemsSymbol, .value = dialogOptions },
          PropPair { .symbol = detailSymbol, .value = std::string("Enter Name of New Scene") },
          PropPair { .symbol = valueSymbol, .value =  newSceneTextData },
          PropPair { .symbol = onInputSymbol, .value = onEdit },
        },
      };
      auto dialogWithProps = withPropsCopy(dialogComponent, dialogProps);
      auto dialogWindow = createUiWindow(dialogWithProps, windowDialogSymbol, []() -> void { windowSetEnabled(windowDialogSymbol, false); }, "New Scene");
      auto defaultProps = getDefaultProps();
      dialogWindow.draw(drawTools, defaultProps);
    }
  }

  {
    FileCallback onFileSelect = [](bool isDirectory, std::string file) -> void {
      modassert(!isDirectory, "on file select gave something not a file");
      modlog("dock - file select", std::string(isDirectory ? "dir" : "file") + " " + file);
      if (onFileAddedFn.has_value()){
        onFileAddedFn.value()(false, file);
      }
    };

    Props filexplorerProps {
      .props = {
        PropPair { .symbol = fileExplorerSymbol, .value = testExplorer },
        PropPair { .symbol = fileChangeSymbol, .value = onFileSelect },
        PropPair { offsetSymbol,  fileexplorerScrollAmount },
      },
    };
    if (fileFilter.has_value()){
      filexplorerProps.props.push_back(PropPair { .symbol = fileFilterSymbol, .value = fileFilter.value() });
    }
    auto fileExplorer = withProps(fileexplorerComponent, filexplorerProps);
    auto fileExplorerWindow = createUiWindow(fileExplorer, windowFileExplorerSymbol, []() -> void { windowSetEnabled(windowFileExplorerSymbol, false); }, "File Explorer");
    auto defaultWindowProps = getDefaultProps();
    fileExplorerWindow.draw(drawTools, defaultWindowProps);
  }

  auto routerProps = createRouterProps(uiContext, selectedId);
  router.draw(drawTools, routerProps);

  {
    Props defaultProps {
      .props = {
        PropPair {
          .symbol = valueSymbol, 
          .value = UtilViewOptions {
            .showKeyboard = uiContext.showKeyboard(),
            .showConsole = uiContext.showConsole(),
            .consoleKeyName = (std::string("console-") + uniqueNameSuffix()),
            .consoleInterface = &uiContext.consoleInterface,
          } 
        },
      },
    };
    utilViewComponent.draw(drawTools, defaultProps);
  }
  
  if (uiContext.showEditor()){
    {
      std::function<void(int)> onImageClick = [](int index) -> void {
        if (onFileAddedFn.has_value()){
          onFileAddedFn.value()(false, imageListDatas.images.at(index).image);
        }
      };

      static bool loadedImages = false;
      if (!loadedImages){
        loadedImages = true;
        auto allTextures = gameapi -> listResources("textures");;
        
        imageListDatas.images = {};
        for (auto &texture : allTextures){
          imageListDatas.images.push_back(ImageListImage {
            .image = texture,
          });
        }
      }

      auto imageListComponent = withPropsCopy(imageList, Props {
        .props = { 
          PropPair { imagesSymbol,  imageListDatas },
          PropPair { onclickSymbol, onImageClick },
          PropPair { offsetSymbol,  imageListScrollAmount },
        }
      });

      {
        Props editorViewProps {
          .props = {
            PropPair {
              .symbol = valueSymbol, 
              .value = EditorViewOptions { 
                .worldPlayInterface = &uiContext.worldPlayInterface,
                .onNewColor = onNewColor,
                .colorPickerTitle = &colorPickerTitle,
                .navbarType = navbarType,
                .onClickNavbar = onClickNavbar,
              } 
            },
          }
        };
        editorViewComponent.draw(drawTools, editorViewProps);
      }


      SceneManagerInterface sceneManagerInterface2 {
        .showScenes = showScenes,
        .offset = offset,
        .onSelectScene = [&uiContext](int index, std::string scene) -> void {
          uiContext.loadScene(scene);
          currentScene = index;
          showScenes = false;
        },
        .toggleShowScenes = []() -> void {
          showScenes = !showScenes;
        },
        .scenes = uiContext.listScenes(),
        .currentScene = currentScene,
      };
      Props sceneManagerProps {
        .props = {
          PropPair { .symbol = valueSymbol, .value = sceneManagerInterface2 },
          PropPair { .symbol = xoffsetSymbol, .value = -0.83f },
          PropPair { .symbol = yoffsetSymbol, .value = 0.9f },
        },
      };
      scenemanagerComponent.draw(drawTools, sceneManagerProps);

      auto uiWindowComponent = createUiWindow(imageListComponent, windowImageExplorerSymbol, []() -> void { windowSetEnabled(windowImageExplorerSymbol, false); }, "Image Explorer");
      auto defaultWindowProps = getDefaultProps();
      uiWindowComponent.draw(drawTools, defaultWindowProps);
    }

 

    // navlist uses this via extern
    uiManagerContext.uiContext = &uiContext;
    auto defaultProps = getDefaultProps();
    withProps(navList, navListProps).draw(drawTools, defaultProps);
  }

  if (uiContext.debugConfig().has_value()){
    Props props {
      .props = {
        PropPair { .symbol = valueSymbol, .value = uiContext.debugConfig().value() },
        PropPair { .symbol = xoffsetSymbol, .value = -1.f },
      },
    };
    debugComponent.draw(drawTools, props);
  }

  if (uiContext.showScreenspaceGrid()){
    drawScreenspaceGrid(ImGrid{ .numCells = 10 });
  }
  getMenuMappingData(&handlerFuncs.minManagedId, &handlerFuncs.maxManagedId);

  if (handlerFuncs.autofocus.has_value()){
    if (lastAutofocusedKey != handlerFuncs.autofocus.value().key){
      focusedId = handlerFuncs.autofocus.value().id;
      lastAutofocusedKey = handlerFuncs.autofocus.value().key;
    }
  }

  if (uiContext.isDebugMode()){
    auto shader = gameapi -> shaderByName("ui");
    drawTools.drawText(std::string("route: ") + fullDebugStr(routerHistory), -0.8f, -0.95f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, ShapeOptions { .shaderId = shader });
    drawTools.drawText(std::string("handlers: ") + std::to_string(handlerFuncs.handlerFns.size()), -0.8f, -0.90f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, ShapeOptions { .shaderId = shader });
    drawTools.drawText(std::string("inputfns: ") + std::to_string(handlerFuncs.inputFns.size()), -0.8f, -0.85f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, ShapeOptions { .shaderId = shader });

    drawTools.drawLine2D(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f), false, glm::vec4(1.f, 1.f, 1.f, 1.f), true, std::nullopt, std::nullopt, ShapeOptions { .shaderId = shader });
    
    //drawTools.drawRect(0.f, 0.f, 0.5f, 0.5f, false, glm::vec4(0.f, 0.f, 1.f, 0.8f), std::nullopt, true, std::nullopt, std::nullopt, ShapeOptions { .shaderId = shader }, std::nullopt);
    drawTools.drawRect(0.5f, 0.5f, 0.5f, 0.5f, false, glm::vec4(0.f, 0.f, 1.f, 0.8f), true, std::nullopt, std::nullopt, ShapeOptions { .zIndex = 1 }, std::nullopt);
    drawTools.drawRect(0.25f, 0.25f, 0.5f, 0.5f, false, glm::vec4(0.f, 1.f, 0.f, 0.8f), true, std::nullopt, std::nullopt, ShapeOptions { .zIndex = 5 }, std::nullopt);
    drawTools.drawRect(0.f, 0.f, 0.5f, 0.5f, false, glm::vec4(1.f, 0.f, 0.f, 0.8f), true, std::nullopt, std::nullopt, ShapeOptions { .zIndex = -1 }, std::nullopt);
  }

  if (ndiCursor.has_value()){
    drawTools.drawRect(ndiCursor.value().x, ndiCursor.value().y, 0.01f, 0.01f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), true, std::nullopt, std::nullopt, ShapeOptions { .zIndex = 6 }, std::nullopt);
  }

  //std::cout << "location data: " << print(handlerFuncs.trackedLocationIds) << std::endl;
  return handlerFuncs;
}

void onMainUiScroll(double amount){
  auto scrollValue = static_cast<int>(amount);
  std::cout << "dock: on main ui scroll: " << scrollValue << std::endl;
  imageListScrollAmount += (scrollValue * 5);
  if (imageListScrollAmount < 0){
    imageListScrollAmount = 0;
  }

  fileexplorerScrollAmount += scrollValue;
  if (fileexplorerScrollAmount < 0){
    fileexplorerScrollAmount = 0;
  }

  offset += scrollValue;
  if (offset < 0){
    offset = 0;
  }
  scenegraphScroll(scrollValue);
}

void onMainUiMousePress(HandlerFns& handlerFns, int button, int action, std::optional<objid> selectedId){
  modassert(handlerFns.minManagedId, "handlerfns minManagedId invalid data");
  modassert(handlerFns.maxManagedId, "handlerfns maxManagedId invalid data");

  std::cout << "button: " << button << ", action: " << action << std::endl;
  if (button == 0 && action == 0){
    windowOnRelease();
    std::cout << uiStoreToStr() << std::endl;
  }
  if (button == 0 && action == 1){
    if (selectedId.has_value() && onGameObjSelected.has_value()){
      auto gameobjName = gameapi -> getGameObjNameForId(selectedId.value()).value();
      onGameObjSelected.value()(selectedId.value(), gameobjName);
      onGameObjSelected = std::nullopt;
    }

    if (selectedId.has_value() &&  selectedId.value() >= handlerFns.minManagedId &&  selectedId.value() <= handlerFns.maxManagedId){
      focusedId = selectedId.value();
    }

    if (selectedId.has_value()){
      if (handlerFns.handlerFns.find(selectedId.value()) != handlerFns.handlerFns.end()){
        handlerFns.handlerFns.at(selectedId.value())();
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

void onMainUiKeyPress(HandlerFns& handlerFns, int key, int scancode, int action, int mods){
  modlog("mainui key press", std::to_string(key));
  if (!focusedId.has_value()){
    return;
  }
  if (handlerFns.inputFns.find(focusedId.value()) != handlerFns.inputFns.end()){
    handlerFns.inputFns.at(focusedId.value())(key, mods);
  }
}

void onMainUiObjectsChanged(){
  refreshScenegraph();
}

void pushHistory(std::vector<std::string> route, bool replace){
  pushHistory(routerHistory, route, replace);
}
void popHistory(){
  popHistory(routerHistory);
}

void pushHistoryParam(std::string param){
  pushHistoryParam(routerHistory, param);
}
void rmHistoryParam(std::string param){
  rmHistoryParam(routerHistory, param);
}

std::string getCurrentPath(){
  return getCurrentPath(routerHistory);
}

std::vector<std::string> historyParams(){
  return historyParams(routerHistory);
}

std::string fullHistoryStr(){
  return fullHistoryStr(routerHistory);
}

std::optional<std::string> getPathParts(int index){
  return getPathParts(routerHistory, index);
}

void sendUiAlert(std::string message){
  modlog("dock alert", message);
  pushAlertMessage(message, ALERT_DETAIL);
}