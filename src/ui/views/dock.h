#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK
#define MOD_AFTERWORLD_COMPONENTS_DOCK

#include "../components/common.h"
#include "../components/basic/list.h"
#include "../components/basic/button.h"
#include "../components/basic/options.h"
#include "../components/slider.h"
#include "../components/basic/checkbox.h"
#include "../components/textbox.h"
#include "../components/scenegraph.h"
#include "../../util.h"

extern Component dockComponent;

struct DockConfigApi {
  std::function<void()> createCamera;
  std::function<void()> createLight;
  std::function<void(std::function<void(bool, std::string)>, std::function<bool(bool, std::string&)>)> openFilePicker;
  std::function<void(std::function<void(bool, std::string)>)> openImagePicker;
  std::function<void(std::function<void(objid, std::string)>)> pickGameObj;
  std::function<void(std::string&)> setTexture;

  std::function<AttributeValue(std::string, std::string)> getAttribute;
  std::function<void(std::string, std::string, AttributeValue value)> setAttribute;

  std::function<std::optional<AttributeValue>(std::string)> getObjAttr;
  std::function<void(std::string, AttributeValue)> setObjAttr;
};

#endif

