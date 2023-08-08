#include "./pausemenu.h"

int pauseMappingIds = 999999;
int mappingId2 = 999995;
std::vector<ImListItem> createPauseMenu(std::function<void()> resume, std::function<void()> goToMainMenu){
  std::vector<ImListItem> listItems;
  listItems.push_back(ImListItem {
    .value = "Resume",
    .onClick = resume,
    .mappingId = pauseMappingIds++,
  });
  listItems.push_back(ImListItem {
    .value = "Main Menu",
    .onClick = goToMainMenu,
    .mappingId = pauseMappingIds++,
  });
  return listItems;
}

const int minwidthSymbol = getSymbol("minwidth");
const int xoffsetSymbol = getSymbol("xoffset");
const int yoffsetSymbol = getSymbol("yoffset");
const int resumeSymbol = getSymbol("resume");
const int pauseSymbol = getSymbol("pause");
const int elapsedTimeSymbol = getSymbol("elapsedTime");

Component createPauseMenuComponent(){
  return Component {
    .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    	auto pauseMenu = createPauseMenu(fnFromProp(props, resumeSymbol).value(), fnFromProp(props, pauseSymbol).value());
      auto minwidth = floatFromProp(props, minwidthSymbol, 0.f);
      auto xoffset = floatFromProp(props, xoffsetSymbol, 0.f);
      auto yoffset = floatFromProp(props, yoffsetSymbol, 0.f);
		  double elapsedTime  = static_cast<double>(floatFromProp(props, elapsedTimeSymbol, 0.f));
		
		  drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt /* texture id */, true, std::nullopt /* selection id */, "./res/textures/testgradient.png");
		  drawTools.drawRect(0.f, 2.f - 2 * glm::min(1.0, elapsedTime / 0.4f), 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 0.8f), std::nullopt /* texture id */, true, std::nullopt /* selection id */, "./res/textures/water.jpg");
		  drawTools.drawRect(0.f, -2.f + 2 * glm::min(1.0, elapsedTime / 0.4f), 2.f, 2.f, false, glm::vec4(0.4f, 0.4f, 0.4f, 0.8f), std::nullopt /* texture id */, true, std::nullopt /* selection id */, "./res/textures/water.jpg");
		  drawTools.drawRect(-2.f + 2 * glm::min(1.0, elapsedTime / 0.4f), 0.f, 1.f, 2.f, false, glm::vec4(1.f, 0.f, 0.f, 0.8f), std::nullopt /* texture id */, true, std::nullopt /* selection id */, "./res/textures/water.jpg");
		  drawTools.drawRect(2.f - 2 * glm::min(1.0, elapsedTime / 0.4f), 0.f, 2.f, 1.f, false, glm::vec4(1.f, 1.f, 1.f, 0.8f), std::nullopt /* texture id */, true, std::nullopt /* selection id */, "./res/textures/water.jpg");
      return drawImMenuList(drawTools, pauseMenu, mappingId2,  xoffset, yoffset, 0.05f /* padding */, 0.015f /* fontsize */, minwidth);
    },
    .imMouseSelect = [](std::optional<objid> mappingIdSelected, Props& props) -> void {
    	 auto pauseMenu = createPauseMenu(fnFromProp(props, resumeSymbol).value(), fnFromProp(props, pauseSymbol).value());
       processImMouseSelect(pauseMenu, mappingIdSelected);
    }  
  };
}