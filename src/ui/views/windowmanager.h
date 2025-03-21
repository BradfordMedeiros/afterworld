#ifndef MOD_AFTERWORLD_COMPONENTS_WINDOWMANAGER
#define MOD_AFTERWORLD_COMPONENTS_WINDOWMANAGER

#include "../components/common.h"
#include "../../global.h"

extern const int windowColorPickerSymbol;
extern const int windowDockSymbol;
extern const int windowFileExplorerSymbol;
extern const int windowImageExplorerSymbol;
extern const int windowDialogSymbol;

struct WindowData {
  glm::vec2 windowOffset;
  std::optional<glm::vec2> initialDragPos;
  bool horizontal;
  bool vertical;
};

extern std::unordered_map<int, WindowData> windowData;

bool windowEnabled(int symbol);
void windowSetEnabled(int symbol, bool enable, glm::vec2 initialPos = glm::vec2(0.f, 0.f));
void windowOnDrag(int symbol);
bool windowIsBeingDragged(int symbol);
void windowOnRelease();
glm::vec2 windowGetPreDragOffset(int symbol);
glm::vec2 windowGetOffset(int symbol);

#endif

