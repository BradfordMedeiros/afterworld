#include "./windowmanager.h"

extern CustomApiBindings* gameapi;


const int windowColorPickerSymbol = getSymbol("window-symbol-colorpicker");
const int windowDockSymbol = getSymbol("window-symbol-dock");

std::map<int, WindowData> windowData = {
  { windowColorPickerSymbol, WindowData {
    .windowOffset = glm::vec2(0.f, 0.f),
    .initialDragPos = std::nullopt,
    .colorPickerOffset = glm::vec2(0.f, 0.f),
    .enable = true,
    .horizontal = true,
    .vertical = true,
  }},
  { windowDockSymbol, WindowData {
    .windowOffset = glm::vec2(0.f, 0.f),
    .initialDragPos = std::nullopt,
    .colorPickerOffset = glm::vec2(0.f, 0.f),
    .enable = true,
    .horizontal = true,
    .vertical = false,
  }}
};

bool windowEnabled(int symbol){
  return windowData.at(symbol).enable;
}
void windowSetEnabled(int symbol, bool enable){
  windowData.at(symbol).enable = enable;
}
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
    if (window.horizontal){
      window.colorPickerOffset.x += getGlobalState().xNdc - window.initialDragPos.value().x;
    }
    if (window.vertical){
      window.colorPickerOffset.y += getGlobalState().yNdc - window.initialDragPos.value().y;
    }
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
  float x = window.horizontal ?  colorPickerOffset.x + draggedOffset.x : colorPickerOffset.x;
  float y = window.vertical  ?  colorPickerOffset.y + draggedOffset.y : colorPickerOffset.y;
  return glm::vec2(x, y);
}
