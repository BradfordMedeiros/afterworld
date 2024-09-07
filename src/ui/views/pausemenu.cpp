#include "./pausemenu.h"

Component pauseMenuComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto pauseMenuPtr = typeFromProps<std::vector<ImListItem>>(props, valueSymbol);
    auto tintPtr = typeFromProps<glm::vec4>(props, tintSymbol);
    auto tint = tintPtr ? *tintPtr : glm::vec4(0.f, 0.f, 0.f, 1.f);

    modassert(pauseMenuPtr, "no valueSymbol for pause menu");
    auto pauseMenu = *pauseMenuPtr;
    auto minwidth = floatFromProp(props, minwidthSymbol, 0.f);
    auto xoffset = floatFromProp(props, xoffsetSymbol, 0.f);
    auto yoffset = floatFromProp(props, yoffsetSymbol, 0.f);
	  double elapsedTime  = static_cast<double>(floatFromProp(props, elapsedTimeSymbol, 0.f));

	  drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(tint.x, tint.y, tint.z, tint.w * interpolateDuration(0.f, 1.f, elapsedTime, 0.2f)), true, std::nullopt /* selection id */, std::nullopt, std::nullopt, std::nullopt);

//	  drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 0.4f), std::nullopt /* texture id */, true, std::nullopt /* selection id */, "./res/textures/testgradient.png", std::nullopt);
	  //drawTools.drawRect(0.f, 2.f - 2 * glm::min(1.0, elapsedTime / 0.4f), 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 0.8f), std::nullopt /* texture id */, true, std::nullopt /* selection id */, "./res/textures/water.jpg", std::nullopt);
	  //drawTools.drawRect(0.f, -2.f + 2 * glm::min(1.0, elapsedTime / 0.4f), 2.f, 2.f, false, glm::vec4(0.4f, 0.4f, 0.4f, 0.8f), std::nullopt /* texture id */, true, std::nullopt /* selection id */, "./res/textures/water.jpg", std::nullopt);
	  //drawTools.drawRect(-2.f + 2 * glm::min(1.0, elapsedTime / 0.4f), 0.f, 1.f, 2.f, false, glm::vec4(1.f, 0.f, 0.f, 0.8f), std::nullopt /* texture id */, true, std::nullopt /* selection id */, "./res/textures/water.jpg", std::nullopt);
	  //drawTools.drawRect(2.f - 2 * glm::min(1.0, elapsedTime / 0.4f), 0.f, 2.f, 1.f, false, glm::vec4(1.f, 1.f, 1.f, 0.8f), std::nullopt /* texture id */, true, std::nullopt /* selection id */, "./res/textures/water.jpg", std::nullopt);
    return drawImMenuList(drawTools, pauseMenu, xoffset, yoffset, 0.05f /* padding */, 0.015f /* fontsize */, minwidth, 0.f /* min height */, glm::vec4(0.f, 0.f, 0.f, 0.f));
  },
};

Props pauseMenuProps(std::optional<objid> mappingId, UiContext& uiContext){
  std::vector<ImListItem> listItems;
  listItems.push_back(ImListItem {
    .value = "Resume",
    .onClick = uiContext.pauseInterface.resume,
    .mappingId = uniqueMenuItemMappingId(),
  });
  listItems.push_back(ImListItem {
    .value = "Main Menu",
    .onClick = uiContext.levels.goToMenu,
    .mappingId = uniqueMenuItemMappingId(),
  });
  Props props {
    .props = {
      { .symbol = elapsedTimeSymbol, .value = uiContext.pauseInterface.elapsedTime() },
      { .symbol = valueSymbol, .value = listItems } ,
      { .symbol = yoffsetSymbol, .value = 0.2f },
    },
  };
  return props;
}

Props deadMenuProps(std::optional<objid> mappingId, UiContext& uiContext){
  std::vector<ImListItem> listItems;
  listItems.push_back(ImListItem {
    .value = "Main Menu",
    .onClick = uiContext.levels.goToMenu,
    .mappingId = uniqueMenuItemMappingId(),
  });
  Props props {
    .props = {
      { .symbol = elapsedTimeSymbol, .value = uiContext.pauseInterface.elapsedTime() },
      { .symbol = valueSymbol, .value = listItems } ,
      { .symbol = yoffsetSymbol, .value = 0.2f },
      { .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 0.f, 0.8f ) },
    },
  };
  return props;
}