#include "./windowmanager.h"

extern CustomApiBindings* gameapi;


const int windowSymbol = getSymbol("window-symbol");
const int windowColorPickerSymbol = getSymbol("window-symbol-colorpicker");

std::map<int, WindowData> windowData = {
  { windowColorPickerSymbol, WindowData {
    .windowOffset = glm::vec2(0.f, 0.f),
    .initialDragPos = std::nullopt,
    .colorPickerOffset = glm::vec2(0.f, 0.f),
  }}
};

void windowOnDrag(int symbol){
  auto position = glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc);
  windowData.at(symbol).initialDragPos = position;
}
bool windowIsBeingDragged(int symbol){
  return windowData.at(symbol).initialDragPos.has_value();
}
void windowOnRelease(){
	for (auto &[id, window] : windowData){
		if (!windowIsBeingDragged(id)){
			continue;
		}
  	window.colorPickerOffset += glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc) - window.initialDragPos.value();
  	window.initialDragPos = std::nullopt;
	}
}
glm::vec2 windowGetPreDragOffset(int symbol){
  WindowData& window = windowData.at(symbol);
  return window.colorPickerOffset;
}
glm::vec2 windowGetOffset(int symbol){
  WindowData& window = windowData.at(symbol);
  glm::vec2 draggedOffset(0.f, 0.f);
  if (windowIsBeingDragged(symbol)){
    draggedOffset = glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc) - window.initialDragPos.value();
  }
  auto colorPickerOffset = window.colorPickerOffset;
  float x = colorPickerOffset.x + draggedOffset.x;
  float y = colorPickerOffset.y + draggedOffset.y;
  return glm::vec2(x, y);
}
