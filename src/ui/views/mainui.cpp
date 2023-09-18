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
      //PropPair { .symbol = tintSymbol,      .value = glm::vec4(0.f, 0.f, 0.f, 0.f) },
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

Props debugListProps { 
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


std::string dialog = "";
std::function<void()> xCallbackFn = []() -> void {
  dialog = "";
};


bool showColorPicker = true;
std::function<void(glm::vec4)> onSlide = [](glm::vec4 value) -> void {
  static glm::vec4* activeColor = static_cast<glm::vec4*>(uiConnect(color));
  *activeColor = value;
};

std::optional<std::function<void(bool closedWithoutNewFile, std::string file)>> onFileAddedFn = std::nullopt;
std::optional<std::function<void(objid, std::string)>> onGameObjSelected = std::nullopt;
std::optional<std::function<bool(bool isDirectory, std::string&)>> fileFilter = std::nullopt;

objid activeSceneId(){
  auto selectedId = gameapi -> selected().at(0);
  auto sceneId = gameapi -> listSceneId(selectedId);
  return sceneId;
}

std::optional<AttributeValue> getWorldState(const char* object, const char* attribute){
  auto worldStates = gameapi -> getWorldState();
  for (auto &worldState : worldStates){
    if (worldState.object == object && worldState.attribute == attribute){
      return worldState.value;
    }
  }
  return std::nullopt;
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
int scenegraphScrollAmount = 0;

std::optional<Scenegraph> scenegraph = std::nullopt;
bool refreshScenegraph = true;

HandlerFns handleDrawMainUi(UiContext& uiContext, std::optional<objid> selectedId){
  HandlerFns handlerFuncs {
    .handlerFns = {},
  };
  std::map<objid, std::function<void(int)>> handlerFns2;
  DrawingTools drawTools {
     .drawText = gameapi -> drawText,
     .drawRect = gameapi -> drawRect,
     .drawLine2D = gameapi -> drawLine2D,
     .registerCallbackFns = [&handlerFuncs](objid id, std::function<void()> fn) -> void {
        handlerFuncs.handlerFns[id] = fn;
     },
     .registerCallbackRightFns = [&handlerFuncs](objid id, std::function<void(int)> fn) -> void {
        handlerFuncs.handlerFns2[id] = fn;
     },
     .selectedId = selectedId,
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


  std::vector<ListComponentData> dialogOptions = {
    ListComponentData {
      .name = "confirm",
      .onClick = []() -> void {
        std::cout << "dialog confirm on click" << std::endl;
      },      
    },
    ListComponentData {
      .name = "quit",
      .onClick = []() -> void {
        std::cout << "dialog on quit" << std::endl;
        exit(1);
      },      
    },
  };

  if (dialog != ""){
    Props dialogProps {
      .props = {
        PropPair { .symbol = listItemsSymbol, .value = dialogOptions },
        PropPair { .symbol = titleSymbol, .value = std::string("mainui title") },
        PropPair { .symbol = detailSymbol, .value = std::string("mainui detail") },
        PropPair { .symbol = onclickSymbol, .value = xCallbackFn },
      },
    };
    dialogComponent.draw(drawTools, dialogProps);
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

  {


    std::function<void(int)> onImageClick = [](int index) -> void {
      if (onFileAddedFn.has_value()){
        onFileAddedFn.value()(false, imageListDatas.images.at(index));
      }
    };

    static bool loadedImages = false;
    if (!loadedImages){
      loadedImages = true;
      imageListDatas.images = gameapi -> listResources("textures");
    }

    auto imageListComponent = withPropsCopy(imageList, Props {
      .props = { 
        PropPair { imagesSymbol,  imageListDatas },
        PropPair { onclickSymbol, onImageClick },
        PropPair { offsetSymbol,  imageListScrollAmount },
      }
    });

    auto uiWindowComponent = createUiWindow(imageListComponent, windowImageExplorerSymbol, "Image Explorer");
    auto defaultWindowProps = getDefaultProps();
    uiWindowComponent.draw(drawTools, defaultWindowProps);
  }

  auto routerProps = createRouterProps(uiContext, selectedId);
  router.draw(drawTools, routerProps);

  if (uiContext.isDebugMode()){
    /// scenegraph
    {
      if (refreshScenegraph){
        scenegraph = createScenegraph();
        refreshScenegraph = false;
      }
      std::function<void(int)> onClick = [](int id) -> void {
        std::set<objid> selectedIds = { id };
        gameapi -> setSelected(selectedIds);
      };

      modassert(scenegraph.has_value(), "scenegraph does not have a value");
      Props scenegraphProps {
        .props = {
          PropPair { .symbol = valueSymbol, .value = &(scenegraph.value()) },
          PropPair { .symbol = onclickSymbol, .value = onClick },
          PropPair { .symbol = offsetSymbol,  .value = scenegraphScrollAmount },
        }
      };

      auto selectedIds = gameapi -> selected();
      
      if (selectedIds.size() > 0){
        int selectedId = selectedIds.at(0);
        scenegraphProps.props.push_back(PropPair { .symbol = selectedSymbol, .value = selectedId });
      }
      scenegraphComponent.draw(drawTools, scenegraphProps);
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
          { onclickSymbol, onClickNavbar }
        }
      };
      navbarComponent.draw(drawTools, navbarProps);
    }
    auto defaultProps = getDefaultProps();
    withProps(debugList, debugListProps).draw(drawTools, defaultProps);
    drawTools.drawText(std::string("route: ") + routerHistory.currentPath, .8f, -0.95f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
    drawTools.drawText(std::string("handlers: ") + std::to_string(handlerFuncs.handlerFns.size()), .8f, -0.90f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
  }
  if (uiContext.showScreenspaceGrid()){
    drawScreenspaceGrid(ImGrid{ .numCells = 10 });
  }
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

  scenegraphScrollAmount += scrollValue;
  if (scenegraphScrollAmount < 0){
    scenegraphScrollAmount = 0;
  }
}

void onMainUiMousePress(std::optional<objid> selectedId){
  if (selectedId.has_value() && onGameObjSelected.has_value()){
    auto gameobjName = gameapi -> getGameObjNameForId(selectedId.value()).value();
    onGameObjSelected.value()(selectedId.value(), gameobjName);
    onGameObjSelected = std::nullopt;
  }
}

void onMainUiMouseRelease(){
  windowOnRelease();
}

void onObjectsChanged(){
  refreshScenegraph = true;
}

void pushHistory(std::string route){
	pushHistory(routerHistory, route);
}

std::string getCurrentPath(){
  return getCurrentPath(routerHistory);
}

void sendUiAlert(std::string message){
  std::cout << "dock: alert: " << message << std::endl;
  pushAlertMessage(message);
}