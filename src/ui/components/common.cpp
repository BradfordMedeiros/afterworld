#include "./common.h"

extern CustomApiBindings* gameapi;


std::string print(BoundingBox2D& box){
  return std::string("x = " + std::to_string(box.x) + ", y = " + std::to_string(box.y) + ", width = " + std::to_string(box.width) + ", height = " + std::to_string(box.height));
}

void drawScreenspaceGrid(ImGrid grid){
  float numLines = grid.numCells - 1;
  float ndiSpacePerLine = 1.f / grid.numCells;

  for (int y = 0; y < numLines; y ++){
    float unitLineNdi = ndiSpacePerLine * (y + 1);
    float ndiY = (unitLineNdi * 2.f) - 1.f;
    gameapi -> drawLine2D(glm::vec3(-1.f, ndiY, 0.f), glm::vec3(1.f, ndiY, 0.f), false, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
    //modlog("drawscreenspace", std::string("draw line: - ") + std::to_string(unitLineNdi));
  }
  for (int x = 0; x < numLines; x ++){
    float unitLineNdi = ndiSpacePerLine * (x + 1);
    float ndiX = (unitLineNdi * 2.f) - 1.f;
    gameapi -> drawLine2D(glm::vec3(ndiX, -1.f, 0.f), glm::vec3(ndiX, 1.f, 0.f), false, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
    //modlog("drawscreenspace", std::string("draw line: - ") + std::to_string(unitLineNdi));
  }
}


const int horizontalSymbol = getSymbol("horizontal");
const int listItemsSymbol = getSymbol("listitems");
const int valueSymbol = getSymbol("value");
const int interfaceSymbol = getSymbol("interface");
const int onclickSymbol = getSymbol("onclick");
const int onclickRightSymbol = getSymbol("onclick-right");
const int onInputSymbol = getSymbol("oninput");
const int layoutSymbol = getSymbol("layout");
const int routerSymbol = getSymbol("router");
const int tintSymbol = getSymbol("tint");
const int minwidthSymbol = getSymbol("minwidth");
const int minheightSymbol = getSymbol("minheight");
const int xoffsetSymbol = getSymbol("xoffset");
const int yoffsetSymbol = getSymbol("yoffset");
const int offsetSymbol = getSymbol("offset");
const int elapsedTimeSymbol = getSymbol("elapsedTime");
const int resumeSymbol = getSymbol("resume");
const int goToMainMenuSymbol = getSymbol("gotoMenu");
const int sliderSymbol = getSymbol("slider");
const int radioSymbol = getSymbol("radio");
const int colorSymbol = getSymbol("color");
const int routerMappingSymbol = getSymbol("router-mapping");
const int paddingSymbol = getSymbol("padding");
const int itemPaddingSymbol = getSymbol("item-padding");
const int alignVertical = getSymbol("align-vertical");
const int flowHorizontal = getSymbol("flow-horizontal");
const int flowVertical = getSymbol("flow-vertical");
const int titleSymbol = getSymbol("title");
const int detailSymbol = getSymbol("detail");
const int xoffsetFromSymbol =  getSymbol("xoffset-from");
const int interpolationSymbol = getSymbol("interpolation");
const int checkedSymbol = getSymbol("checked");
const int editableSymbol = getSymbol("editable");
const int onWindowDragSymbol = getSymbol("on-window-drag");
const int onSlideSymbol = getSymbol("on-slide");
const int enableSymbol = getSymbol("enable");
const int dockTypeSymbol = getSymbol("dock-type");
const int selectedSymbol = getSymbol("selected");
const int fontsizeSymbol = getSymbol("fontsize");
const int fixedSizeSymbol = getSymbol("fixed-size");
const int sizeSymbol = getSymbol("size");
const int limitSymbol = getSymbol("limit");
const int focusTintSymbol = getSymbol("focus-tint");
const int borderColorSymbol = getSymbol("border-color");
const int barColorSymbol = getSymbol("bar-color");
const int consoleInterfaceSymbol = getSymbol("console-interface");
const int playLevelSymbol = getSymbol("play-level");
const int widthSymbol = getSymbol("width");
const int heightSymbol = getSymbol("height");
const int autofocusSymbol = getSymbol("autofocus");
