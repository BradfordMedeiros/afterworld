#ifndef MOD_AFTERWORLD_COMPONENTS_EDITORVIEW
#define MOD_AFTERWORLD_COMPONENTS_EDITORVIEW

#include "../components/common.h"
#include "../components/worldplay.h"
#include "../components/colorpicker.h"
#include "../components/uiwindow.h"
#include "../components/fileexplorer.h"
#include "./navlist.h"
#include "./navbar.h"

struct EditorViewOptions {
  WorldPlayInterface* worldPlayInterface;
  std::optional<std::function<void(glm::vec4)>> onNewColor;
  std::string* colorPickerTitle;

  NavbarType navbarType;
  std::function<void(const char*)> onClickNavbar;

  std::optional<std::function<void(bool closedWithoutNewFile, std::string file)>> onFileAddedFn;

  int fileexplorerScrollAmount;
  std::optional<std::function<bool(bool isDirectory, std::string&)>> fileFilter;
};
extern Component editorViewComponent;

#endif

