#include "./mainui.h"

const int routerSymbol = getSymbol("router");
const int routerMappingSymbol = getSymbol("router-mapping");
auto routerHistory = createHistory("playing");

const int tintSymbol = getSymbol("tint");


std::map<std::string, Component> routeToComponent = {
  { "playing",  createList(
    { ListComponentData { .name = "playing", .onClick = []() -> void {
        pushHistory("paused");
      }},
      ListComponentData { .name = "playing", .onClick = []() -> void {
        pushHistory("quit");
      }}
    }
  )},
  { "paused",   createList({
    { ListComponentData { .name = "paused", .onClick = std::nullopt }}
  })  },
  { "quit",  createList(
    { ListComponentData { .name = "quit", .onClick = []() -> void {
      exit(0);
    }}}
  )},
  { "",   createList({
    { ListComponentData { .name = "default", .onClick = std::nullopt }}
  })  },
};

Component mainUI = createRouter();


Props pauseMenuProps(std::optional<objid> mappingId, PauseContext pauseContext){
  Props props {
    .mappingId = mappingId,
    .props = {
      { .symbol = getSymbol("elapsedTime"), .value = pauseContext.elapsedTime },
      { .symbol = getSymbol("pause"), .value = pauseContext.pause } ,
      { .symbol = getSymbol("resume"), .value = pauseContext.resume },
      { .symbol = getSymbol("yoffset"), .value = 0.2f },
    },
  };
  return props;
}

Props createRouterProps(std::optional<objid> selectedId){
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

void handleDrawMainUi(PauseContext& pauseContext, DrawingTools& drawTools, std::optional<objid> selectedId){
   auto routerProps = createRouterProps(selectedId);
   mainUI.draw(drawTools, routerProps);

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
  nestedListTestComponent.draw(drawTools, nestedListProps);

  if (pauseContext.shouldShowPauseMenu){
    auto props = pauseMenuProps(selectedId, pauseContext);
    createPauseMenuComponent().draw(drawTools, props);    
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
  if (pauseContext.showAnimationMenu){
    drawImMenuList(drawTools, animationMenuItems2(), selectedId, 1.5f /*xoffset*/, 0.2f /*yoffset*/ , 0.05f, 0.015f, 0.f /* minwidth */);
  }

  if (pauseContext.showScreenspaceGrid){
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

  imageListTest.draw(drawTools, imageProps);

  Props& sliderProps = getSliderProps(selectedId);
  sliderSelector.draw(drawTools, sliderProps);
}

void handleInputMainUi(PauseContext& pauseContext, std::optional<objid> selectedId){
  auto routerProps = createRouterProps(selectedId);
  mainUI.imMouseSelect(selectedId, routerProps);
  Props nestedListProps { 
    .mappingId = selectedId, 
    .props = {}
  };
  nestedListTestComponent.imMouseSelect(selectedId, nestedListProps);
  processImMouseSelect(animationMenuItems2(), selectedId);

  if (pauseContext.onMainMenu){
    // should render main menu items
    //processImMouseSelect(mainMenuItems2(gameState), mappingId);
  }

  Props& sliderProps = getSliderProps(selectedId);
  sliderSelector.imMouseSelect(selectedId, sliderProps);
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

const int minwidthSymbol = getSymbol("minwidth");
const int xoffsetSymbol = getSymbol("xoffset");
const int yoffsetSymbol = getSymbol("yoffset");

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
