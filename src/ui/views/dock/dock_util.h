#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK_UTIL
#define MOD_AFTERWORLD_COMPONENTS_DOCK_UTIL

#include "../../../util.h"
#include "../../components/common.h"
#include "../style.h"

struct DockConfigApi {
  std::function<void(std::string&)> createMesh;
  std::function<void()> createCamera;
  std::function<void()> createLight;
  std::function<void()> createNavmesh;
  std::function<void(std::function<void(bool, std::string)>, std::function<bool(bool, std::string&)>)> openFilePicker;
  std::function<void(std::function<void(bool, std::string)>)> openImagePicker;
  std::function<void(std::function<void(glm::vec4)>, std::string)> openColorPicker;
  std::function<void(std::function<void(objid, std::string)>)> pickGameObj;
  std::function<void(std::string&)> setTexture;

  std::function<AttributeValue(std::string, std::string)> getAttribute;
  std::function<void(std::string, std::string, AttributeValue value)> setAttribute;

  std::function<std::optional<AttributeValue>(std::string)> getObjAttr;
  std::function<void(std::string, AttributeValue)> setObjAttr;
  std::function<void(std::string&)> setEditorBackground;

  std::function<void()> emitParticleViewerParticle;
  std::function<void(bool)> setParticlesViewerShouldEmit;
  std::function<bool()> getParticlesViewerShouldEmit;
  std::function<void(std::string, AttributeValue)> setParticleAttribute;
  std::function<std::optional<AttributeValue>(std::string)> getParticleAttribute;

  std::function<void()> saveScene;
  std::function<void()> resetScene;
};

std::optional<glm::vec2> toVec2(std::string& text);
std::optional<glm::vec3> toVec3(std::string& text);
std::optional<glm::vec4> toVec4(std::string& text);

std::optional<float> toNumber(std::string& text);
std::optional<float> toPositiveNumber(std::string& text);
std::optional<int> toInteger(std::string& text);
std::optional<int> toPositiveInteger(std::string& text);

std::function<bool()> getIsCheckedWorld(std::string key, std::string attribute, std::string enabledValue, std::string disabledValue);
std::function<void(bool)> getOnCheckedWorld(std::string key, std::string attribute, std::string enabledValue, std::string disabledValue);
std::function<bool()> getIsCheckedGameobj(std::string key, std::string enabledValue, std::string disabledValue);
std::function<void(bool)> getOnCheckedGameobj(std::string key, std::string enabledValue, std::string disabledValue);
std::function<void(float)> getSetFloatGameobj(std::string key);
std::function<float()> getFloatGameobj(std::string key);
std::function<void(std::string&)> getStrGameObj(const char* attribute);

struct OptionData {
  const char* optionName;
};
std::function<void(std::string& choice, int)> optionsOnClick(std::string key, std::string attribute, std::vector<AttributeValue> optionValueMapping);
std::function<int()> optionsSelectedIndex(std::string key, std::string attribute, std::vector<AttributeValue> optionValueMapping);
std::function<void(std::string& choice, int)> optionsOnClickObj(std::string attribute, std::vector<AttributeValue> optionValueMapping);
std::function<int()> optionsSelectedIndexObj(std::string attribute, std::vector<AttributeValue> optionValueMapping);

#endif

