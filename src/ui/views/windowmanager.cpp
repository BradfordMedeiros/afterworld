#include "./windowmanager.h"

extern CustomApiBindings* gameapi;


const int windowColorPickerSymbol = getSymbol("window-symbol-colorpicker");
const int windowDockSymbol = getSymbol("window-symbol-dock");
const int windowFileExplorerSymbol = getSymbol("window-fileexplorer");
const int windowImageExplorerSymbol = getSymbol("window-imageexplorer");

std::map<int, WindowData> windowData = {
  { windowColorPickerSymbol, WindowData {
    .windowOffset = glm::vec2(0.f, 0.f),
    .initialDragPos = std::nullopt,
    .enable = false,
    .horizontal = true,
    .vertical = false,
  }},
  { windowDockSymbol, WindowData {
    .windowOffset = glm::vec2(1.f, 0.88f),
    .initialDragPos = std::nullopt,
    .enable = false,
    .horizontal = true,
    .vertical = true,
  }},
  { windowFileExplorerSymbol, WindowData {
    .windowOffset = glm::vec2(0.f, 0.f),
    .initialDragPos = std::nullopt,
    .enable = false,
    .horizontal = true,
    .vertical = true,
  }},
  { windowImageExplorerSymbol, WindowData {
    .windowOffset = glm::vec2(0.f, 0.f),
    .initialDragPos = std::nullopt,
    .enable = false,
    .horizontal = true,
    .vertical = true,
  }},
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

