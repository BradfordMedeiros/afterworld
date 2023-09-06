#ifndef MOD_AFTERWORLD_COMPONENTS_WINDOWMANAGER
#define MOD_AFTERWORLD_COMPONENTS_WINDOWMANAGER

#include "../components/common.h"
#include "../../global.h"

extern const int windowSymbol;
extern const int windowColorPickerSymbol;
struct WindowData {
  glm::vec2 windowOffset;
  std::optional<glm::vec2> initialDragPos;
  glm::vec2 colorPickerOffset;
};

extern std::map<int, WindowData> windowData;

void windowOnDrag(int symbol);
bool windowIsBeingDragged(int symbol);
void windowOnRelease();
glm::vec2 windowGetPreDragOffset(int symbol);
glm::vec2 windowGetOffset(int symbol);

#endif

