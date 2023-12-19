#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK_TEXTBOX
#define MOD_AFTERWORLD_COMPONENTS_DOCK_TEXTBOX

#include "../dock_util.h"
#include "../../../components/basic/textbox.h"
#include "../../../components/basic/list.h"

struct DockTextboxConfig {
  std::string label;
  std::function<std::string(void)> text;
  std::function<void(std::string)> onEdit;
};

Component createDockTextbox(DockTextboxConfig& textboxOptions);

#endif

