#ifndef MOD_AFTERWORLD_COMPONENTS_EDITORVIEW
#define MOD_AFTERWORLD_COMPONENTS_EDITORVIEW

#include "../components/common.h"
#include "../components/worldplay.h"
#include "../components/colorpicker.h"
#include "../components/uiwindow.h"
#include "./navbar.h"

struct EditorViewOptions {
  WorldPlayInterface* worldPlayInterface;
  std::optional<std::function<void(glm::vec4)>> onNewColor;
  std::string* colorPickerTitle;

  NavbarType navbarType;
  std::function<void(const char*)> onClickNavbar;
};
extern Component editorViewComponent;

#endif

