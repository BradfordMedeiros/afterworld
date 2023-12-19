#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK_TEXTBOX_NUMERIC
#define MOD_AFTERWORLD_COMPONENTS_DOCK_TEXTBOX_NUMERIC

#include "../dock_util.h"
#include "../../../components/basic/textbox.h"

struct DockTextboxNumeric {
  std::string label;
  float value;
};

Component createDockTextboxNumeric(DockTextboxNumeric& dockTextboxNumeric);

#endif

