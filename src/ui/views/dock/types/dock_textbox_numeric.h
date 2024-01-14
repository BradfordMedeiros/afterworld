#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK_TEXTBOX_NUMERIC
#define MOD_AFTERWORLD_COMPONENTS_DOCK_TEXTBOX_NUMERIC

#include "../dock_util.h"
#include "../../../components/basic/textbox.h"
#include "../../../components/basic/list.h"

struct DockTextboxNumeric {
  std::string label;
  std::function<std::string()> value;
  std::function<void(float, std::string&)> onEdit;
};

Component createDockTextboxNumeric(DockTextboxNumeric& dockTextboxNumeric);

#endif

