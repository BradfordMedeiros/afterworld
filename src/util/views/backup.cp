   auto defaultProps = getDefaultProps(selectedId);
   auto routerProps = createRouterProps(selectedId);
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

  if (pauseContext.shouldShowPauseMenu){
    auto props = pauseMenuProps(selectedId, pauseContext);
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



//////////////////////////
/// handle input
//////////////////////////
//////////////////////////


    auto routerProps = createRouterProps(selectedId);
  router.imMouseSelect(selectedId, routerProps);
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
  slider.imMouseSelect(selectedId, sliderProps);