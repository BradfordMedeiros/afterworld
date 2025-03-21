#include "./windowmanager.h"

extern CustomApiBindings* gameapi;

const int windowColorPickerSymbol = getSymbol("window-symbol-colorpicker");
const int windowFileExplorerSymbol = getSymbol("window-fileexplorer");
const int windowImageExplorerSymbol = getSymbol("window-imageexplorer");
const int windowDialogSymbol = getSymbol("window-dialogsymbol");

std::unordered_map<int, WindowData> windowData = {};

bool windowEnabled(int symbol){
  return windowData.find(symbol) != windowData.end();
}
void windowSetEnabled(int symbol, bool enable, glm::vec2 initialPos){
  if (enable){
    if (windowData.find(symbol) == windowData.end()){
      windowData[symbol] = WindowData {
        .windowOffset = initialPos,
        .initialDragPos = std::nullopt,
        .horizontal = true,
        .vertical = true,
      };
    }
  }else{
    windowData.erase(symbol);
  }

}
void windowOnDrag(int symbol){
  auto position = glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc);
  windowData.at(symbol).initialDragPos = position;
}
bool windowIsBeingDragged(int symbol){
  return windowData.at(symbol).initialDragPos.has_value();
}
void windowOnRelease(){
  modlog("window", "release");
	for (auto &[id, window] : windowData){
		if (!windowIsBeingDragged(id)){
			continue;
		}
    if (window.horizontal){
      window.windowOffset.x += getGlobalState().xNdc - window.initialDragPos.value().x;
    }
    if (window.vertical){
      window.windowOffset.y += getGlobalState().yNdc - window.initialDragPos.value().y;
    }
  	window.initialDragPos = std::nullopt;
	}
}
glm::vec2 windowGetPreDragOffset(int symbol){
  WindowData& window = windowData.at(symbol);
  return window.windowOffset;
}
glm::vec2 windowGetOffset(int symbol){
  WindowData& window = windowData.at(symbol);
  glm::vec2 draggedOffset(0.f, 0.f);
  if (windowIsBeingDragged(symbol)){
    draggedOffset = glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc) - window.initialDragPos.value();
  }
  auto windowOffset = window.windowOffset;
  float x = window.horizontal ?  windowOffset.x + draggedOffset.x : windowOffset.x;
  float y = window.vertical  ?  windowOffset.y + draggedOffset.y : windowOffset.y;
  return glm::vec2(x, y);
}

