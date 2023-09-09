#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK
#define MOD_AFTERWORLD_COMPONENTS_DOCK

#include "../components/common.h"
#include "../components/list.h"
#include "../components/button.h"
#include "../components/options.h"
#include "../components/slider.h"
#include "../components/checkbox.h"
#include "../components/textbox.h"

extern Component dockComponent;

struct DockConfigApi {
  std::function<void()> createCamera;
  std::function<void()> createLight;
  std::function<void(std::function<void(bool, std::string)>)> openFilePicker;
  std::function<void(std::function<void(bool, std::string)>)> openImagePicker;
};

#endif

