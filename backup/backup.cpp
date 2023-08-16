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


  if (uiContext.showAnimationMenu){
    drawImMenuList(drawTools, animationMenuItems2(), 1.5f /*xoffset*/, 0.2f /*yoffset*/ , 0.05f, 0.015f, 0.f /* minwidth */);
  }

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
