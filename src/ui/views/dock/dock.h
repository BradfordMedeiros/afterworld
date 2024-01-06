#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK
#define MOD_AFTERWORLD_COMPONENTS_DOCK

#include "../../components/common.h"
#include "../../components/basic/list.h"
#include "../../components/basic/button.h"
#include "../../components/basic/options.h"
#include "../../components/basic/slider.h"
#include "../../components/basic/checkbox.h"
#include "../../components/basic/textbox.h"
#include "../../components/scenegraph.h"
#include "../../../util.h"
#include "./dock_util.h"
#include "./types/dock_label.h"
#include "./types/dock_button.h"
#include "./types/dock_options.h"
#include "./types/dock_slider.h"
#include "./types/dock_textbox.h"
#include "./types/dock_textbox_numeric.h"
#include "./types/dock_checkbox.h"
#include "./types/dock_image.h"
#include "./types/dock_file.h"
#include "./types/dock_gameobj.h"
#include "./types/dock_colorpicker.h"
#include "./types/dock_select.h"

void persistSqlFloat(std::string column, float value);


struct DockScenegraph {};
struct DockGroup;

typedef std::variant<
  DockLabelConfig,   DockButtonConfig, DockOptionConfig, DockSliderConfig,    DockCheckboxConfig, 
  DockTextboxConfig, DockFileConfig,   DockImageConfig,  DockGameObjSelector, DockGroup, 
  DockScenegraph, DockTextboxNumeric, DockColorPickerConfig, DockSelectConfig
  > DockConfig;

struct DockGroup {
  std::string groupName;
  std::function<void()> onClick;  
  std::function<bool()> collapse;
  std::vector<DockConfig> configFields;
};

struct DockConfiguration {
  std::string title;
  std::vector<DockConfig> configFields;
};

extern Component dockComponent;
extern Component dockFormComponent;

#endif

