#include "./mainui.h"

extern CustomApiBindings* gameapi;

const int routerSymbol = getSymbol("router");
const int routerMappingSymbol = getSymbol("router-mapping");
auto routerHistory = createHistory("mainmenu");

const int tintSymbol = getSymbol("tint");
const int listItemsSymbol = getSymbol("listitems");
const int minwidthSymbol = getSymbol("minwidth");
const int xoffsetSymbol = getSymbol("xoffset");
const int yoffsetSymbol = getSymbol("yoffset");
const int radioSymbol  = getSymbol("radio");


Props createLevelListProps(UiContext& uiContext){
  std::vector<ListComponentData> levels;
  auto levelDatas = uiContext.levels.getLevels();

  for (auto &levelData : levelDatas){
    levels.push_back(ListComponentData {
      .name = levelData.name,
      .onClick = [&uiContext, levelData]() -> void {
        auto level = levelData;
        uiContext.levels.goToLevel(level);
        uiContext.pauseInterface.pause();
      }
    });
  }
  Props levelProps {
    .props = {
      { listItemsSymbol, levels },
    },
  };
  return levelProps;

}

Props pauseMenuProps(std::optional<objid> mappingId, UiContext& uiContext){
  Props props {
    .props = {
      { .symbol = getSymbol("elapsedTime"), .value = uiContext.pauseInterface.elapsedTime },
      { .symbol = getSymbol("pause"), .value = uiContext.pauseInterface.pause } ,
      { .symbol = getSymbol("resume"), .value = uiContext.pauseInterface.resume },
      { .symbol = getSymbol("yoffset"), .value = 0.2f },
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
  .props = {
    { sliderSymbol, exampleSlider },
  },
};

Props& getSliderProps(){
  return sliderProps;  
}



std::map<objid, std::function<void()>> handleDrawMainUi(UiContext& uiContext, std::optional<objid> selectedId){
   std::map<objid, std::function<void()>> handlerFns;
   DrawingTools drawTools {
     .drawText = gameapi -> drawText,
     .drawRect = gameapi -> drawRect,
     .drawLine2D = gameapi -> drawLine2D,
     .registerCallbackFns = [&handlerFns](objid id, std::function<void()> fn) -> void {
        //modassert(handlerFns.find(id) == handlerFns.end(), "duplicate hander function");
        //handlerFns[id] = fn;
        handlerFns[id] = [id, fn]() -> void {
          std::cout << std::string("test handler fn for : ") + std::to_string(id) << std::endl;
          fn();
        };
     },
     .selectedId = selectedId,
   };
   resetMenuItemMappingId();

   auto defaultProps = getDefaultProps(selectedId);
   auto routerProps = createRouterProps(uiContext, selectedId);
   router.draw(drawTools, routerProps);

   Props nestedListProps { 
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


  if (uiContext.showAnimationMenu){
    drawImMenuList(drawTools, animationMenuItems2(), 1.5f /*xoffset*/, 0.2f /*yoffset*/ , 0.05f, 0.015f, 0.f /* minwidth */);
  }

  if (uiContext.showScreenspaceGrid()){
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
    .props = {
      { getSymbol("images"), defaultImages },
    }
  };

 

  auto images = withProps(imageList, imageProps);
  //images.draw(drawTools, defaultProps);
  //imageListTest.draw(drawTools, imageProps);

  Props& sliderProps = getSliderProps();
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
    .props = {
      PropPair { .symbol = xoffsetSymbol, .value = 0.5f },
      PropPair { .symbol = yoffsetSymbol, .value = 0.5f },
      PropPair { .symbol = radioSymbol, .value = radioButtonContainer },
    }
  };
  radioButtons.draw(drawTools, radioProps);
  return handlerFns;
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
