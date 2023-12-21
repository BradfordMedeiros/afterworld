#include "./mainui.h"

extern CustomApiBindings* gameapi;

auto routerHistory = createHistory("mainmenu");

Props createLevelListProps(UiContext& uiContext){
  std::vector<ListComponentData> levels;
  auto levelDatas = uiContext.levels.getLevels();

  for (auto &levelData : levelDatas){
    levels.push_back(ListComponentData {
      .name = levelData.name,
      .onClick = [&uiContext, levelData]() -> void {
        auto level = levelData;
        uiContext.levels.goToLevel(level);
      }
    });
  }
  Props levelProps {
    .props = {
      PropPair { .symbol = listItemsSymbol, .value = levels },
      PropPair { .symbol = xoffsetSymbol,   .value = -0.9f },
      PropPair { .symbol = yoffsetSymbol,   .value = 0.2f },
      PropPair { .symbol = tintSymbol,      .value = glm::vec4(0.f, 0.f, 0.f, 0.3f) },
      PropPair { .symbol = flowHorizontal,  .value = UILayoutFlowPositive2 },
      PropPair { .symbol = flowVertical,    .value = UILayoutFlowNegative2 },
    },
  };
  return levelProps;
}

Props pauseMenuProps(std::optional<objid> mappingId, UiContext& uiContext){
  Props props {
    .props = {
      { .symbol = elapsedTimeSymbol, .value = uiContext.pauseInterface.elapsedTime },
      { .symbol = goToMainMenuSymbol, .value = uiContext.levels.goToMenu } ,
      { .symbol = resumeSymbol, .value = uiContext.pauseInterface.resume },
      { .symbol = yoffsetSymbol, .value = 0.2f },
    },
  };
  return props;
}

Props createRouterProps(UiContext& uiContext, std::optional<objid> selectedId){
  auto props = pauseMenuProps(selectedId, uiContext);
  auto pauseComponent = withPropsCopy(pauseMenuComponent, props);

  std::map<std::string, Component> routeToComponent = {
    { "mainmenu",  withPropsCopy(listComponent, createLevelListProps(uiContext)) },
    { "playing",  emptyComponent },
    { "paused", pauseComponent },
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

Props navListProps { 
  .props = {
    PropPair {
      .symbol = tintSymbol,
      .value = glm::vec4(0.f, 0.f, 0.f, 0.8f),
    },
    PropPair {
      .symbol = minwidthSymbol,
      .value = 0.15f,
    },
    PropPair {
      .symbol = xoffsetSymbol,
      .value = -0.99f,
    },
    PropPair {
      .symbol = yoffsetSymbol,
      .value = 0.98f,  
    }
  }
};

std::string dockedDock = "";
std::function<void(const char*)> onClickNavbar = [](const char* value) -> void {
  //pushQueryParam(routerHistory, "dockedDock");
  dockedDock = value;
  modlog("dock", std::string("new dock: ") + dockedDock);
  if (dockedDock == ""){
    windowSetEnabled(windowDockSymbol, false);
  }else{
    windowSetEnabled(windowDockSymbol, true);
  }
};


bool showColorPicker = true;
std::function<void(glm::vec4)> onSlide = [](glm::vec4 value) -> void {
  static glm::vec4* activeColor = static_cast<glm::vec4*>(uiConnect(color));
  *activeColor = value;
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
objid activeSceneId(){
  modassert(uiManagerContext.uiContext, "uicontext null - active scene");
  auto activeScene = uiManagerContext.uiContext -> activeSceneId();
  modassert(activeScene.has_value(), "no active scene");
  return activeScene.value();
}

DockConfigApi dockConfigApi { // probably should be done via a prop for better control flow
  .createCamera = []() -> void {
    std::map<std::string, GameobjAttributes> submodelAttributes;
    GameobjAttributes attr {
      .stringAttributes = {},
      .numAttributes = {},
      .vecAttr = {  .vec3 = {},  .vec4 = {} },
    };
    gameapi -> makeObjectAttr(activeSceneId(), std::string(">camera-") + uniqueNameSuffix(), attr, submodelAttributes);
  },
  .createLight = []() -> void {
    std::map<std::string, GameobjAttributes> submodelAttributes;
    GameobjAttributes attr {
      .stringAttributes = {},
      .numAttributes = {},
      .vecAttr = {  .vec3 = {},  .vec4 = {} },
    };
    gameapi -> makeObjectAttr(activeSceneId(), std::string("!light-") + uniqueNameSuffix(), attr, submodelAttributes);
  },
  .createNavmesh = []() -> void {
    std::map<std::string, GameobjAttributes> submodelAttributes;
    GameobjAttributes attr {
      .stringAttributes = {},
      .numAttributes = {},
      .vecAttr = {  .vec3 = {},  .vec4 = {} },
    };
    gameapi -> makeObjectAttr(activeSceneId(), std::string(";navmesh-") + uniqueNameSuffix(), attr, submodelAttributes);
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
      GameobjAttributes attr {
        .stringAttributes = {
          { "texture", texture },
        },
        .numAttributes = {},
        .vecAttr = {
          .vec3 = {},
          .vec4 = {},
        },
      };
      gameapi -> setGameObjectAttr(id, attr);     
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
    auto gameobjAttr =  gameapi -> getGameObjectAttr(id);
    auto attr = getAttr(gameobjAttr, key);
    return attr;
  },
  .setObjAttr = [](std::string key, AttributeValue value) -> void {
    auto selected = gameapi -> selected();
    if (selected.size() == 0){
      return;
    }
    objid id = selected.at(0);
    std::map<std::string, std::string> stringAttributes;
    std::map<std::string, double> numAttributes;
    std::map<std::string, glm::vec3> vec3Attributes;
    std::map<std::string, glm::vec4> vec4Attributes;

    auto strAttr = std::get_if<std::string>(&value);
    if (strAttr){
      std::cout << "setting string attr: " << key << ", " << *strAttr << std::endl;
      stringAttributes[key] = (*strAttr);
    }
    auto numAttr = std::get_if<float>(&value);
    if (numAttr){
      numAttributes[key] = (*numAttr);
    }
    auto vec3Attr = std::get_if<glm::vec3>(&value);
    if (vec3Attr){
      vec3Attributes[key] = (*vec3Attr);
    }
    auto vec4Attr = std::get_if<glm::vec4>(&value);
    if (vec4Attr){
      vec4Attributes[key] = (*vec4Attr);
    }
    GameobjAttributes newAttr {
      .stringAttributes = stringAttributes,
      .numAttributes = numAttributes,
      .vecAttr = { 
        .vec3 = vec3Attributes, 
        .vec4 = vec4Attributes, 
      },
    };
    gameapi -> setGameObjectAttr(id, newAttr);
  }

};

ImageList imageListDatas {
  .images = {},
};
int imageListScrollAmount = 0;
int fileexplorerScrollAmount = 0;

NavbarType navbarType = MAIN_EDITOR;
NavListApi navListApi {
  .changeLayout = [](std::string layout) -> void {
    if (layout == "main"){
      navbarType = MAIN_EDITOR;
    }else if (layout == "gameplay"){
      navbarType = GAMEPLAY_EDITOR;
    }else{
      modassert(false, std::string("invalid layout: ") + layout);
    }
  }
};


bool showScenes = false;
int offset = 2;
int currentScene = -1;

std::optional<objid> focusedId = std::nullopt;
TextData newSceneTextData {
  .valueText = "",
  .cursorLocation = 0,
  .highlightLength = 0,
  .maxchars = -1,
};


std::string print(std::unordered_map<objid, TrackedLocationData>& trackedLocationIds){
  std::string value = "[";
  for (auto &[id, data] : trackedLocationIds){
    value += "(" + print(data.position) + ", " + print(data.size) + ") ";
  }
  return value + "]";
}



HandlerFns handleDrawMainUi(UiContext& uiContext, std::optional<objid> selectedId){
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
     .drawText = gameapi -> drawText,
     .getTextDimensionsNdi = gameapi -> getTextDimensionsNdi,
     .drawRect = [&handlerFuncs](float centerX, float centerY, float width, float height, bool perma, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture, std::optional<objid> trackingId) -> void {
      if (trackingId.has_value()){
        handlerFuncs.trackedLocationIds[trackingId.value()] = TrackedLocationData {
          .position = glm::vec2(centerX, centerY),
          .size = glm::vec2(width, height),
        };
      }
      gameapi -> drawRect(centerX, centerY, width, height, perma, tint, textureId, ndi, selectionId, texture);
     },
     .drawLine2D = gameapi -> drawLine2D,
     .registerCallbackFns = [&handlerFuncs](objid id, std::function<void()> fn) -> void {
        handlerFuncs.handlerFns[id] = fn;
     },
     .registerCallbackFnsHandler = [&handlerFuncs](objid id, std::function<void(HandlerCallbackFn&)> fn) -> void {
        handlerFuncs.handlerCallbackFns[id] = fn;
     },
     .registerCallbackRightFns = [&handlerFuncs](objid id, std::function<void(int)> fn) -> void {
        handlerFuncs.handlerFns2[id] = fn;
     },
     .registerInputFns = [&handlerFuncs](objid id, std::function<void(int)> fn) -> void {
        handlerFuncs.inputFns[id] = fn;
     },
     .selectedId = selectedId,
     .focusedId = focusedId,
  };
  resetMenuItemMappingId();

  if (dockedDock != ""){
    Props dockProps { 
      .props = {
        { dockTypeSymbol, dockedDock }, 
        { xoffsetFromSymbol, 1.5f },
      }
    };
    auto dock = withProps(dockComponent, dockProps);
    auto defaultWindowProps = getDefaultProps();
    createUiWindow(dock, windowDockSymbol, dockedDock, AlignmentParams { .layoutFlowHorizontal = UILayoutFlowNegative2, .layoutFlowVertical = UILayoutFlowNegative2 }).draw(drawTools, defaultWindowProps);
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

   //////////////////////////////


    std::function<void(TextData)> onEdit = [](TextData textData) -> void {
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
    auto dialogWindow = createUiWindow(dialogWithProps, windowDialogSymbol, "New Scene");

    auto defaultProps = getDefaultProps();
    dialogWindow.draw(drawTools, defaultProps);
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
    auto fileExplorerWindow = createUiWindow(fileExplorer, windowFileExplorerSymbol, "File Explorer");
    auto defaultWindowProps = getDefaultProps();
    fileExplorerWindow.draw(drawTools, defaultWindowProps);
  }


  {
    static glm::vec4* activeColor = static_cast<glm::vec4*>(uiConnect(color));
    auto colorPicker = withPropsCopy(colorPickerComponent, Props {
      .props = { 
        PropPair { onSlideSymbol,  onSlide },
        PropPair { tintSymbol, *activeColor },
      }
    });
    auto uiWindowComponent = createUiWindow(colorPicker, windowColorPickerSymbol, "color picker");
    auto defaultWindowProps = getDefaultProps();
    uiWindowComponent.draw(drawTools, defaultWindowProps);
  }



  auto routerProps = createRouterProps(uiContext, selectedId);
  router.draw(drawTools, routerProps);

  if (uiContext.isDebugMode()){
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

      Props worldPlayProps {
        .props = {
          PropPair { .symbol = xoffsetSymbol, .value = 0.f },
          PropPair { .symbol = yoffsetSymbol, .value = -1.f },
          PropPair { .symbol = valueSymbol, .value = &uiContext.worldPlayInterface },
        }
      };
      worldplay.draw(drawTools, worldPlayProps);

      SceneManagerInterface sceneManagerInterface2 {
        .showScenes = showScenes,
        .offset = offset,
        .onSelectScene = [&uiContext, &currentScene](int index, std::string scene) -> void {
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

      auto uiWindowComponent = createUiWindow(imageListComponent, windowImageExplorerSymbol, "Image Explorer");
      auto defaultWindowProps = getDefaultProps();
      uiWindowComponent.draw(drawTools, defaultWindowProps);
    }
    {
      Props defaultProps {
        .props = {},
      };
      alertComponent.draw(drawTools, defaultProps);
    }
    {
      Props navbarProps { 
        .props = {
          { onclickSymbol, onClickNavbar },
          { valueSymbol, navbarType },
        }
      };
      navbarComponent.draw(drawTools, navbarProps);
    }

    // navlist uses this via extern
    uiManagerContext.uiContext = &uiContext;
    auto defaultProps = getDefaultProps();
    withProps(navList, navListProps).draw(drawTools, defaultProps);

    if (uiContext.showConsole()){
      auto consoleDefaultProps = getDefaultProps();
      consoleComponent.draw(drawTools, consoleDefaultProps);      
    }

    //auto weaponWheelProps = getDefaultProps();
    //weaponWheelComponent.draw(drawTools, weaponWheelProps);
    //auto compassProps = getDefaultProps();
    //compassComponent.draw(drawTools, compassProps);

    drawTools.drawText(std::string("route: ") + routerHistory.currentPath, .8f, -0.95f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
    drawTools.drawText(std::string("handlers: ") + std::to_string(handlerFuncs.handlerFns.size()), .8f, -0.90f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
    drawTools.drawText(std::string("inputfns: ") + std::to_string(handlerFuncs.inputFns.size()), .8f, -0.85f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
  }
  if (uiContext.showScreenspaceGrid()){
    drawScreenspaceGrid(ImGrid{ .numCells = 10 });
  }
  getMenuMappingData(&handlerFuncs.minManagedId, &handlerFuncs.maxManagedId);

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

void onMainUiKeyPress(HandlerFns& handlerFns, int key){
  modlog("mainui key press", std::to_string(key));
  if (!focusedId.has_value()){
    return;
  }
  if (handlerFns.inputFns.find(focusedId.value()) != handlerFns.inputFns.end()){
    handlerFns.inputFns.at(focusedId.value())(key);
  }
}


void onObjectsChanged(){
  refreshScenegraph();
}

void pushHistory(std::string route){
	pushHistory(routerHistory, route);
}

std::string getCurrentPath(){
  return getCurrentPath(routerHistory);
}

void sendUiAlert(std::string message){
  std::cout << "dock: alert: " << message << std::endl;
  pushAlertMessage(message, ALERT_DETAIL);
}