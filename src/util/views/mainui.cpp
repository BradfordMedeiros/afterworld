#include "./mainui.h"

const int routerSymbol = getSymbol("router");
const int routerMappingSymbol = getSymbol("router-mapping");
auto routerHistory = createHistory("mainmenu");

const int tintSymbol = getSymbol("tint");
const int listItemsSymbol = getSymbol("listitems");
const int minwidthSymbol = getSymbol("minwidth");
const int xoffsetSymbol = getSymbol("xoffset");
const int yoffsetSymbol = getSymbol("yoffset");
const int radioSymbol  = getSymbol("radio");

std::vector<ListComponentData> playingListItems = { 
  ListComponentData { 
    .name = "playing wow", 
    .onClick = []() -> void {
      pushHistory("paused");
    }
  },
  ListComponentData { 
    .name = "playing woah", 
    .onClick = []() -> void {
      pushHistory("quit");
    }
  }
};
Props playingListProps {
  .mappingId = std::nullopt,
  .props = {
    { listItemsSymbol, playingListItems },
  },
};

Props createLevelListProps(UiContext& uiContext){
  std::vector<ListComponentData> levels;
  auto levelDatas = uiContext.levels.getLevels();
  for (auto &levelData : levelDatas){
    levels.push_back(ListComponentData {
      .name = levelData.name,
      .onClick = [&uiContext, levelData]() -> void {
        auto level = levelData;
        uiContext.levels.goToLevel(level);
        pushHistory("playing");
      }
    });
  }
  Props levelProps {
    .mappingId = std::nullopt,
    .props = {
      { listItemsSymbol, levels },
    },
  };
  return levelProps;

}

std::vector<ListComponentData> quitListItems = { 
  ListComponentData { 
    .name = "quit", 
    .onClick = []() -> void {
      exit(0);
    }
  },
};
Props quitProps {
  .mappingId = std::nullopt,
  .props = {
    { listItemsSymbol, quitListItems },
  },
};


Props pauseMenuProps(std::optional<objid> mappingId, UiContext uiContext){
  Props props {
    .mappingId = mappingId,
    .props = {
      { .symbol = getSymbol("elapsedTime"), .value = uiContext.elapsedTime },
      { .symbol = getSymbol("pause"), .value = uiContext.pause } ,
      { .symbol = getSymbol("resume"), .value = uiContext.resume },
      { .symbol = getSymbol("yoffset"), .value = 0.2f },
    },
  };
  return props;
}

Props createRouterProps(UiContext& uiContext, std::optional<objid> selectedId){
  std::map<std::string, Component> routeToComponent = {
    { "mainmenu",  withPropsCopy(listComponent, createLevelListProps(uiContext)) },
    { "playing",  emptyComponent },
    { "quit",  withProps(listComponent, quitProps) },
    { "",  withProps(listComponent, playingListProps)  },
  };

  Props routerProps {
    .mappingId = selectedId,
    .props = {
      { routerSymbol, routerHistory },
      { routerMappingSymbol, routeToComponent },
      { tintSymbol, glm::vec4(1.f, 1.f, 1.f, 0.2f) }
    },
  };
  return routerProps;
}

Slider exampleSlider {
  .min = 0.f,
  .max = 10.f,
  .percentage = 0.f,
  .mappingId = 34545,
  .update = false,
};
const int sliderSymbol = getSymbol("slider");
Props sliderProps {
  .mappingId = std::nullopt,
  .props = {
    { sliderSymbol, exampleSlider },
  },
};

Props& getSliderProps(std::optional<objid> selectedId){
  sliderProps.mappingId = selectedId;
  return sliderProps;  
}



void handleDrawMainUi(UiContext& uiContext, DrawingTools& drawTools, std::optional<objid> selectedId){
   auto defaultProps = getDefaultProps(selectedId);
   auto routerProps = createRouterProps(uiContext, selectedId);
   router.draw(drawTools, routerProps);

   Props nestedListProps { 
    .mappingId = selectedId, 
    .props = {
      PropPair {
        .symbol = getSymbol("tint"),
        .value = glm::vec4(0.f, 0.f, 0.f, 0.8f),
      },
      PropPair {
        .symbol = getSymbol("minwidth"),
        .value = 0.15f,
      },
      PropPair {
        .symbol = getSymbol("xoffset"),
        .value = -0.99f,
      },
      PropPair {
        .symbol = getSymbol("yoffset"),
        .value = 0.98f,  
      }
    }
  };
  withProps(nestedListTestComponent, nestedListProps).draw(drawTools, defaultProps);

  if (uiContext.shouldShowPauseMenu){
    auto props = pauseMenuProps(selectedId, uiContext);
    pauseMenuComponent.draw(drawTools, props);    
  }

  /* 
  show main menu items 
  if (!gameState -> loadedLevel.has_value()){
    std::vector<ListComponentData> levels;
    for (auto &level : gameState -> levels){
      levels.push_back(ListComponentData{
        .name = level.name,
        .onClick = std::nullopt,
      });
    }
  }
  */
  /*drawDebugBoundingBox(*/ //);
  if (uiContext.showAnimationMenu){
    drawImMenuList(drawTools, animationMenuItems2(), selectedId, 1.5f /*xoffset*/, 0.2f /*yoffset*/ , 0.05f, 0.015f, 0.f /* minwidth */);
  }

  if (uiContext.showScreenspaceGrid){
    drawScreenspaceGrid(ImGrid{ .numCells = 10 });
  }

  ImageList defaultImages {
    .images = {
      "./res/textures/wood.jpg",
      "./res/textures/grass.jpg",
      "./res/textures/brickwall.jpg",
      "./res/textures/grid.png",
      "./res/textures/grass.jpg",
      "./res/textures/grid.png",
      "./res/textures/wood.jpg",
      "./res/textures/brickwall.jpg",
    },
  };
  Props imageProps { 
    .mappingId = selectedId, 
    .props = {
      { getSymbol("images"), defaultImages },
    }
  };

 

  auto images = withProps(imageList, imageProps);
  //images.draw(drawTools, defaultProps);
  //imageListTest.draw(drawTools, imageProps);

  Props& sliderProps = getSliderProps(selectedId);
  slider.draw(drawTools, sliderProps);

  RadioButtonContainer radioButtonContainer {
    .selectedRadioButtonIndex = 0,
    .radioButtons = {
      RadioButton {
        .selected = false,
        .hovered = false,
        .onClick = std::nullopt,
        .mappingId = std::nullopt,
      },
      RadioButton {
        .selected = true,
        .hovered = false,
        .onClick = std::nullopt,
        .mappingId = std::nullopt,
      },
    },
  };

  Props radioProps {
    .mappingId = std::nullopt,
    .props = {
      PropPair { .symbol = xoffsetSymbol, .value = 0.5f },
      PropPair { .symbol = yoffsetSymbol, .value = 0.5f },
      PropPair { .symbol = radioSymbol, .value = radioButtonContainer },
    }
  };
  radioButtons.draw(drawTools, radioProps);
}

void handleInputMainUi(UiContext& uiContext, std::optional<objid> selectedId){
  auto routerProps = createRouterProps(uiContext, selectedId);
  router.imMouseSelect(selectedId, routerProps);
  Props nestedListProps { 
    .mappingId = selectedId, 
    .props = {}
  };
  nestedListTestComponent.imMouseSelect(selectedId, nestedListProps);
  processImMouseSelect(animationMenuItems2(), selectedId);

  if (uiContext.onMainMenu){
    // should render main menu items
    //processImMouseSelect(mainMenuItems2(gameState), mappingId);
  }

  Props& sliderProps = getSliderProps(selectedId);
  slider.imMouseSelect(selectedId, sliderProps);
}

void pushHistory(std::string route){
	pushHistory(routerHistory, route);
}

//

extern CustomApiBindings* gameapi;
std::vector<ImListItem> animationMenuItems2(){
  int mappingId = 900000;
  auto selectedIds = gameapi -> selected();
  if (selectedIds.size() == 0){
    return { ImListItem { .value = "no object selected" , .onClick = std::nullopt, mappingId = mappingId }};
  }
  auto selectedId = selectedIds.at(0);
  std::vector<ImListItem> items;
  for (auto &animation : gameapi -> listAnimations(selectedId)){
    items.push_back(ImListItem{
      .value = animation,
      .onClick = [selectedId, animation]() -> void {
        gameapi -> playAnimation(selectedId, animation, LOOP);
      },
      .mappingId = mappingId++,
    });
  }
  if (items.size() == 0){
    items.push_back(ImListItem {
      .value = "no animations",
      .onClick = std::nullopt,
      .mappingId = mappingId++,
    });
  }
  return items;
}


std::vector<Component> mainMenuItems2(){
  std::vector<Component> elements;

  /*int mappingId = 90000;
  for (int i = 0; i < gameState.levels.size(); i++){
    ImListItem menuItem {
      .value = gameState.levels.at(i).name,
      .onClick = [&gameState, i]() -> void {
        goToLevel(gameState, gameState.levels.at(i).scene);
      },
      .mappingId = mappingId++,
    };
    elements.push_back(
      Component {
        .draw = [menuItem](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
          float padding = 0.05f;
          auto minwidth = floatFromProp(props, minwidthSymbol, 0.f);
          float xoffset = floatFromProp(props, xoffsetSymbol, 0.f);
          float yoffset = floatFromProp(props, yoffsetSymbol, 0.f);

          auto box = drawImMenuListItem(drawTools, menuItem, props.mappingId, xoffset, yoffset, padding, 0.015f , minwidth);
          drawDebugBoundingBox(drawTools, box);
          return box;
        },
        .imMouseSelect = [menuItem](std::optional<objid> mappingIdSelected, Props& props) -> void {
          if (mappingIdSelected.has_value() && mappingIdSelected.value() == menuItem.mappingId.value()){
            if (menuItem.onClick.has_value()){
              menuItem.onClick.value()();
            }
          }
        },
      }
    );
  }*/
  return elements;
}
