#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK_COLORPICKER
#define MOD_AFTERWORLD_COMPONENTS_DOCK_COLORPICKER

#include "../dock_util.h"
#include "../../../components/basic/listitem.h"

struct DockColorPickerConfig {
  std::string label;
  std::function<glm::vec4()> getColor;
  std::function<void(glm::vec4)> onColor;
};

Component createDockColorPicker(DockColorPickerConfig& dockColorPicker);

#endif

