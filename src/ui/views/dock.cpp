#include "./dock.h"

extern CustomApiBindings* gameapi;
extern DockConfigApi dockConfigApi;

const float STYLE_UI_DOCK_ELEMENT_PADDING = 0.02f;

struct DockLabelConfig {
  std::string label;
};

struct DockButtonConfig {
  const char* buttonText;
  std::function<void()> onClick;
};
struct DockOptionConfig {
  std::vector<const char*> options;
  std::function<void(std::string&, int)> onClick;
  std::function<int(void)> getSelectedIndex;
};
struct DockSliderConfig {
  std::string label;
  float min;
  float max;
  std::function<float()> percentage;
  std::function<void(float)> onSlide;
};
struct DockCheckboxConfig {
  std::string label;
  std::function<bool()> isChecked;
  std::function<void(bool)> onChecked;
};
struct DockTextboxConfig {
  std::string label;
  std::function<std::string(void)> text;
  std::function<void(std::string)> onEdit;
};

struct DockFileConfig {
  std::string label;
};

struct DockImageConfig {
  std::string label;
  std::function<void(std::string)> onImageSelect;
};

struct DockGameObjSelector {
  std::optional<std::string> label;
  std::function<void(std::string&)> onSelect;
};

struct DockScenegraph {};

struct DockTextboxNumeric {
  std::string label;
  float value;
};

struct DockImageGroup;

typedef std::variant<DockLabelConfig, DockButtonConfig, DockOptionConfig, DockSliderConfig, DockCheckboxConfig, DockTextboxConfig, DockFileConfig, DockImageConfig, DockGameObjSelector, DockImageGroup, DockScenegraph, DockTextboxNumeric> DockConfig;

struct DockImageGroup {
  std::string groupName;
  std::function<void()> onClick;  
  std::function<bool()> collapse;
  std::vector<DockConfig> configFields;
};

struct DockConfiguration {
  std::string title;
  std::vector<DockConfig> configFields;
};

int currentDebugMask(){
  auto value = dockConfigApi.getAttribute("editor", "debugmask");
  auto debugMaskFloat = std::get_if<float>(&value);
  modassert(debugMaskFloat, "debug mask is not a float");
  auto debugValue = static_cast<int>(*debugMaskFloat);
  return debugValue; 
}
std::function<bool()> getIsDebugMaskEnabled(int bitmask){
  return [bitmask]() -> bool {
    auto debugValue = currentDebugMask();
    auto newMask = debugValue & bitmask;
    //std::cout << "debug value is: " << debugValue << ", check against: " << bitmask << ", new mask: " << newMask << std::endl;
    return newMask != 0;
  };
}

std::function<void(bool)> getOnDebugMaskEnabled(int bitmask){
  return [bitmask](bool checked) -> void {
    auto debugMask = currentDebugMask();
    if (checked){
      debugMask = debugMask | bitmask; // add the bit to the old mask
    }else{
      debugMask = debugMask & ~(bitmask);   // take the bit away from the old mask, by making all 1s except the bit
    }
    auto floatMask = static_cast<float>(debugMask);
    dockConfigApi.setAttribute("editor", "debugmask", floatMask);
  };
}

std::unordered_map<std::string, bool> collapseValues;
std::function<void()> createCollapsableOnClick(const char* value){
  collapseValues[value] = true;
  return [value]() -> void { collapseValues[value] = !collapseValues.at(value); };
}
std::function<bool()> createShouldBeCollapse(const char* value){
  return [value]() -> bool { return collapseValues.at(value); };
}

std::map<std::string, std::string> textStore;
enum TextEditType { 
  TEXT_TYPE_STRING, 
  TEXT_TYPE_NUMBER, TEXT_TYPE_POSITIVE_NUMBER, TEXT_TYPE_INTEGER, TEXT_TYPE_POSITIVE_INTEGER,
  TEXT_TYPE_VEC2, TEXT_TYPE_VEC3, TEXT_TYPE_VEC4
};

std::function<std::string()> connectGetText(std::string key){
  if (textStore.find(key) == textStore.end()){
    textStore[key] = "";
  }
  return [key]() -> std::string {
    return textStore.at(key);
  };
}

std::function<std::string()> connectGetTextVec2(std::string key){
  if (textStore.find(key) == textStore.end()){
    textStore[key] = "";
  }
  return [key]() -> std::string {
    auto attr = dockConfigApi.getObjAttr(key);
    if (!attr.has_value()){
      return "";
    }
    auto value = std::get_if<std::string>(&attr.value());
    if (!value){
      return "";
    }
    return *value;
  };
}

std::function<void(std::string)> connectEditTextVec2(std::string key, const char* objKey){
  return [key, objKey](std::string value) -> void {
    if (value.size() == 0){
      textStore[key] = value;
    }
    auto vec2Value = toVec2(value);
    std::cout << "setting vec2Value: " << (vec2Value.has_value() ? "true" : "false") << std::endl;
    if (vec2Value.has_value()){
      textStore[key] = value;

      std::string asString = std::to_string(vec2Value.value().x) + " " + std::to_string(vec2Value.value().y);
      dockConfigApi.setObjAttr(objKey, asString);
    }
  };
}


std::function<void(std::string)> connectEditText(std::string key, TextEditType type = TEXT_TYPE_STRING){
  return [key, type](std::string value) -> void {
    if (value.size() == 0){
      textStore[key] = value;
      return;
    }
    if (type == TEXT_TYPE_STRING){
      textStore[key] = value;
    }else if (type == TEXT_TYPE_NUMBER && toNumber(value).has_value()){
      textStore[key] = value;
    }else if (type == TEXT_TYPE_POSITIVE_NUMBER && toPositiveNumber(value).has_value()){
      textStore[key] = value;
    }else if (type == TEXT_TYPE_INTEGER && toInteger(value).has_value()){
      textStore[key] = value;
    }else if (type == TEXT_TYPE_POSITIVE_INTEGER && toPositiveInteger(value).has_value()){
      textStore[key] = value;
    }else if (type == TEXT_TYPE_VEC2 && toVec2(value).has_value()){
      textStore[key] = value;
    }else if (type == TEXT_TYPE_VEC3 && toVec3(value).has_value()){
      textStore[key] = value;
    }else if (type == TEXT_TYPE_VEC4 && toVec4(value).has_value()){
      textStore[key] = value;
    }
  };
}

std::vector<DockConfiguration> configurations {
  DockConfiguration {
    .title = "",
    .configFields = {
      DockButtonConfig {
        .buttonText = "no panel available",
        .onClick = []() -> void {},
      },
    },
  },
  DockConfiguration {
    .title = "Object Details",
    .configFields = {
      DockTextboxNumeric {
        .label = "position x",
        .value = 1.f,
      },
      DockTextboxNumeric {
        .label = "position y",
        .value = 1.f,
      },
      DockTextboxNumeric {
        .label = "position z",
        .value = 1.f,
      },
      DockTextboxNumeric {
        .label = "scale x",
        .value = 1.f,
      },
      DockTextboxNumeric {
        .label = "scale y",
        .value = 1.f,
      },
      DockTextboxNumeric {
        .label = "scale z",
        .value = 1.f,
      },
    },
  },
  DockConfiguration {
    .title = "Textures",
    .configFields = {
      DockTextboxConfig {
        .label = "Tiling",
        .text = connectGetTextVec2("texturetiling"),
        .onEdit = connectEditTextVec2("tiling-x", "texturetiling"),
      },
      DockTextboxConfig {
        .label = "Size",
        .text = connectGetTextVec2("texturesize"),
        .onEdit = connectEditTextVec2("texturesize", "texturesize"),
      },
      DockTextboxConfig {
        .label = "Offset",
        .text = connectGetTextVec2("textureoffset"),
        .onEdit = connectEditTextVec2("textureoffset", "textureoffset"),
      },
    },
  },
  DockConfiguration {
    .title = "Editor",
    .configFields = {
      DockCheckboxConfig {
        .label = "Group Selection",
        .isChecked = getIsCheckedWorld("editor", "groupselection", "true", "false"),
        .onChecked = getOnCheckedWorld("editor", "groupselection", "true", "false"),
      },
      DockCheckboxConfig {
        .label = "Symmetric Translate",
        .isChecked = getIsCheckedWorld("tools", "position-mirror", "true", "false"),
        .onChecked = getOnCheckedWorld("tools", "position-mirror", "true", "false"),
      },
      DockCheckboxConfig {
        .label = "Absolute Translate",
        .isChecked = getIsCheckedWorld("tools", "snap-position", "absolute", "relative"),
        .onChecked = getOnCheckedWorld("tools", "snap-position", "absolute", "relative"),
      },
      DockOptionConfig { // Snap Translates
        .options = { "0.01", "0.1", "0.5", "1", "5" },
        .onClick = optionsOnClick("editor", "snaptranslate-index", { 0.0, 1.0, 2.0, 3.0, 4.0 }),
        .getSelectedIndex = optionsSelectedIndex("editor", "snaptranslate-index", { 0.0, 1.0, 2.0, 3.0, 4.0 }),
      },
      DockCheckboxConfig {
        .label = "Preserve Scale",
        .isChecked = getIsCheckedWorld("tools", "preserve-scale", "true", "false"),
        .onChecked = getOnCheckedWorld("tools", "preserve-scale", "true", "false"),
      },
      DockOptionConfig {  // "Snap Scales",
        .options = { "0.01", "0.1", "0.5", "1", "5" },
        .onClick = optionsOnClick("editor", "snapscale-index", { 0.0, 1.0, 2.0, 3.0, 4.0 }),
        .getSelectedIndex = optionsSelectedIndex("editor", "snapscale-index", { 0.0, 1.0, 2.0, 3.0, 4.0 }),
      },
      DockCheckboxConfig {
        .label = "Absolute Rotation",
        .isChecked = getIsCheckedWorld("tools", "snap-rotate", "absolute", "relative"),
        .onChecked = getOnCheckedWorld("tools", "snap-rotate", "absolute", "relative"),
      },
      DockOptionConfig { // Snap Rotation
        .options = { "1", "5", "15", "30", "45", "90", "180" },
        .onClick = optionsOnClick("editor", "snapangle-index", { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 }),
        .getSelectedIndex = optionsSelectedIndex("editor", "snapangle-index", { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 }),
      },
    },
  },
  DockConfiguration {
    .title = "Cameras",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Create Camera",
        .onClick = []() -> void { dockConfigApi.createCamera(); },
      },
      DockOptionConfig {
        .options = { "enable dof", "disable dof" },
        .onClick = optionsOnClickObj("dof", { "enabled", "disabled" }),
        .getSelectedIndex = optionsSelectedIndexObj("dof",  { "enabled", "disabled" }),
      },
      DockCheckboxConfig {
        .label = "enable physics",
        .isChecked = getIsCheckedGameobj("physics", "enabled", "disabled"),
        .onChecked = getOnCheckedGameobj("physics", "enabled", "disabled"),
      },    

      DockImageGroup {
        .groupName = "Depth of Field Blur",
        .onClick = createCollapsableOnClick("blur"),
        .collapse = createShouldBeCollapse("blur"),
        .configFields = {
          DockGameObjSelector {
            .label = std::nullopt,
            .onSelect =  getStrGameObj("target"),
          },
          DockCheckboxConfig {
            .label = "toggle dof",
            .isChecked = getIsCheckedGameobj("dof", "enabled", "disabled"),
            .onChecked = getOnCheckedGameobj("dof", "enabled", "disabled"),
          },
          DockSliderConfig {
            .label = "blur min",
            .min = 0.f,
            .max = 1.f,
            .percentage = getFloatGameobj("minblur"),
            .onSlide = getSetFloatGameobj("minblur"),
          },
          DockSliderConfig {
            .label = "blur max",
            .min = 0.f,
            .max = 1.f,
            .percentage = getFloatGameobj("maxblur"),
            .onSlide = getSetFloatGameobj("maxblur"),
          },
          DockSliderConfig {
            .label = "blur amount",
            .min = 0.f,
            .max = 1.f,
            .percentage = getFloatGameobj("bluramount"),
            .onSlide = getSetFloatGameobj("bluramount"),
          },
        },
      },
    },
  },
  DockConfiguration {
    .title = "Lights",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Create Light",
        .onClick = []() -> void { dockConfigApi.createLight(); } ,
      },
      DockOptionConfig {
        .options = { "point", "spotlight", "directional" },
        .onClick = optionsOnClickObj("type", { "point", "spotlight", "directional" }),
        .getSelectedIndex = optionsSelectedIndexObj("type", { "point", "spotlight", "directional" }),
      },
      DockTextboxConfig {
        .label = "Test Vec2",
        .text = connectGetText("test-text"),
        .onEdit = connectEditText("test-text", TEXT_TYPE_VEC2 ),
      },
      DockTextboxConfig {
        .label = "Test Vec",
        .text = connectGetText("test-text"),
        .onEdit = connectEditText("test-text"),
      },

      DockFileConfig {
        .label = "somefile-here",
      },
      DockImageConfig {
        .label =  "someimage-here",
        .onImageSelect = [](std::string texture) -> void {
          dockConfigApi.setTexture(texture);
        }
      },
    },
  },
  DockConfiguration {
    .title = "Scenegraph",
    .configFields = {
      DockScenegraph {},
    },
  },
  DockConfiguration {
    .title = "Debug",
    .configFields = {
      DockCheckboxConfig {
        .label = "Show Debug",
        .isChecked = getIsCheckedWorld("editor", "debug", "true", "false"),
        .onChecked = getOnCheckedWorld("editor", "debug", "true", "false"),
      },

      //     if(objectValue.object == "editor" && objectValue.attribute == "debugmask"){
      //   debugValue = maskBasedOnFloatField(details, debugValue, "debug-show-cameras", 0b10);
      //debugValue = maskBasedOnFloatField(details, debugValue, "debug-show-sound", 0b100);
      //debugValue = maskBasedOnFloatField(details, debugValue, "debug-show-lights", 0b1000);
      DockCheckboxConfig {
        .label = "Show Cameras",
        .isChecked = getIsDebugMaskEnabled(0b10),
        .onChecked = getOnDebugMaskEnabled(0b10),
      },
      DockCheckboxConfig {
        .label = "Show Lights",
        .isChecked = getIsDebugMaskEnabled(0b1000),
        .onChecked = getOnDebugMaskEnabled(0b1000),
      },
      DockCheckboxConfig {
        .label = "Show Sound",
        .isChecked = getIsDebugMaskEnabled(0b100),
        .onChecked = getOnDebugMaskEnabled(0b100),
      },   
    },
  },
  DockConfiguration {
    .title = "Transform",
    .configFields = {
      DockOptionConfig { // Snap Translates
        .options = { "translate", "scale", "rotate" },
        .onClick = optionsOnClick("tools", "manipulator-mode", { "translate", "scale", "rotate" }),
        .getSelectedIndex = optionsSelectedIndex("tools", "manipulator-mode", { "translate", "scale", "rotate" }),
      },
      DockOptionConfig { // Snap Translates
        .options = { "x", "y", "z" },
        .onClick = optionsOnClick("tools", "manipulator-axis", { "x", "y", "z" }),
        .getSelectedIndex = optionsSelectedIndex("tools", "manipulator-axis", { "x", "y", "z" }),
      },
    }
  },
  DockConfiguration {
    .title = "Navmesh",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Create Navmesh",
        .onClick = []() -> void { dockConfigApi.createNavmesh(); } ,
      },
    }
  },

  // Gameplay Docks //////////////////////////////////////////////
  //////////////////////////////////////////////////////////////
  DockConfiguration {
    .title = "MOVEMENT",
    .configFields = {
      DockButtonConfig {
        .buttonText = "MOVEMENT",
        .onClick = []() -> void {},
      },
    }
  },
  DockConfiguration {
    .title = "WEAPONS",
    .configFields = {
      DockCheckboxConfig {
        .label = "Iron Sights",
        .isChecked = getIsCheckedWorld("editor", "groupselection", "true", "false"),
        .onChecked = getOnCheckedWorld("editor", "groupselection", "true", "false"),
      },
      DockTextboxNumeric {
        .label = "Horizontal Sway",
        .value = 10.f,
        // gameobj:water-viscosity  // positive number
      },
      DockTextboxNumeric {
        .label = "Vertical Sway",
        .value = 10.f,
        // gameobj:water-viscosity  // positive number
      },
    }
  },
  DockConfiguration {
    .title = "TRIGGERS",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Create Trigger",
        .onClick = []() -> void {},
      },
      DockTextboxConfig {
        .label = "Trigger",
        .text = []() -> std::string {  // gameobj:trigger-switch
          return "trigger-name-plaeholder"; 
        },
        .onEdit = [](std::string value) -> void {

        }
      },
    }
  },
  DockConfiguration {
    .title = "HUD",
    .configFields = {
      DockButtonConfig {
        .buttonText = "HUD",
        .onClick = []() -> void {},
      },
    }
  },
  DockConfiguration {
    .title = "WATER",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Create Water",
        .onClick = []() -> void {},
      },
      DockTextboxNumeric {
        .label = "Density",
        .value = 10.f,
        // gameobj:water-density   // positive number
      },
      DockTextboxNumeric {
        .label = "Viscosity",
        .value = 10.f,
        // gameobj:water-viscosity  // positive number
      },
      DockTextboxNumeric {
        .label = "Gravity",
        .value = 10.f,
        // gameobj:water-gravity  // positive number
      },
    }
  },
};

DockConfiguration* dockConfigByName(std::string name){
  for (DockConfiguration& config : configurations){
    if (config.title == name){
      return &config;
    }
  }
  return NULL;
}

Component textboxWithLabel = wrapWithLabel(textbox);

void componentsForFields(std::vector<DockConfig>& configFields, std::vector<Component>& elements);
Component createDockComponent(DockConfig& config){
  auto dockLabel = std::get_if<DockLabelConfig>(&config);
  if (dockLabel){
    Props textboxProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = dockLabel -> label },
      }
    };
    auto textboxWithProps = withPropsCopy(textbox, textboxProps);
    return textboxWithProps;  
  }

  auto dockButton = std::get_if<DockButtonConfig>(&config);
  if (dockButton){
    Props buttonProps {
      .props = {
        PropPair { .symbol = paddingSymbol, .value = STYLE_UI_DOCK_ELEMENT_PADDING },
        PropPair { .symbol = valueSymbol, .value = std::string(dockButton -> buttonText) },
        PropPair { .symbol = onclickSymbol, .value =  dockButton -> onClick }
      }
    };
    return withPropsCopy(button, buttonProps);  
  }

  auto dockOptions = std::get_if<DockOptionConfig>(&config);
  if (dockOptions){
    std::vector<Option> optionsData = {};
    for (int i = 0; i < dockOptions -> options.size(); i++){
      auto option = dockOptions -> options.at(i);
      auto onClick = dockOptions -> onClick;
      optionsData.push_back(Option {
        .name = option,
        .onClick = [onClick, option, i]() -> void {
          std::string optionStr(option);
          onClick(optionStr, i);
        },
      });
    }
    Options defaultOptions {
      .options = optionsData,
      .selectedIndex = dockOptions -> getSelectedIndex(),
    };
    Props optionsProps {
      .props = {
        PropPair { .symbol = optionsSymbol, .value = defaultOptions },
        PropPair { .symbol = itemPaddingSymbol, .value = STYLE_UI_DOCK_ELEMENT_PADDING },
      }
    };
    return withPropsCopy(options, optionsProps);
  }

  auto sliderOptions = std::get_if<DockSliderConfig>(&config);
  if (sliderOptions){
    Slider sliderData {
      .min = sliderOptions -> min,
      .max = sliderOptions -> max,
      .percentage = sliderOptions -> percentage(),
      .onSlide = sliderOptions -> onSlide,
    };
    Props sliderProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = sliderOptions -> label },
        PropPair { .symbol = sliderSymbol, .value = sliderData },
      },
    };
    auto sliderWithProps = withPropsCopy(slider, sliderProps); 
    return sliderWithProps;
  }

  auto checkboxOptions = std::get_if<DockCheckboxConfig>(&config);
  if (checkboxOptions){
    Props checkboxProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = checkboxOptions -> label },
        PropPair { .symbol = checkedSymbol, .value = checkboxOptions -> isChecked() },
        PropPair { .symbol = onclickSymbol, .value = checkboxOptions -> onChecked },
      },
    };
    auto checkboxWithProps = withPropsCopy(checkbox, checkboxProps);
    return checkboxWithProps;
  }

  auto textboxOptions = std::get_if<DockTextboxConfig>(&config);
  if (textboxOptions){
    static TextData textboxConfigData {
      .valueText = "somedebugtext",
      .cursorLocation = 0,
      .highlightLength = 0,
      .maxchars = -1,
    };

    std::function<void(TextData)> onEdit = [textboxOptions](TextData textData) -> void {
      textboxConfigData = textData;
      textboxOptions -> onEdit(textData.valueText);
    };

    TextData textData {
      .valueText = textboxOptions -> text(),
      .cursorLocation = textboxConfigData.cursorLocation,
      .highlightLength = textboxConfigData.highlightLength,
      .maxchars = textboxConfigData.maxchars,
    };

    Props textboxProps {
      .props = {
        PropPair { .symbol = editableSymbol, .value = true },
        PropPair { .symbol = textDataSymbol, .value = textData },
        PropPair { .symbol = onInputSymbol, .value = onEdit },
        PropPair { .symbol = valueSymbol, .value = textboxOptions -> label },
      }
    };
    auto textboxWithProps = withPropsCopy(textboxWithLabel, textboxProps);
    return textboxWithProps;
  }

  auto fileconfigOptions = std::get_if<DockFileConfig>(&config);
  if (fileconfigOptions){
    std::function<void()> onClick =  [fileconfigOptions]() -> void {
      dockConfigApi.openFilePicker([fileconfigOptions](bool justClosed, std::string file) -> void {
        std::cout << "open file picker dialog mock: " << justClosed << ", file = " << file << std::endl;
        if (!justClosed){
          fileconfigOptions -> label = file;
        }
      }, [](bool isDirectory, std::string&) -> bool { return !isDirectory; });
    };
    Props textboxProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = fileconfigOptions -> label },
        PropPair { .symbol = onclickSymbol, .value = onClick },
      }
    };
    auto textboxWithProps = withPropsCopy(textbox, textboxProps);
    return textboxWithProps; 
  }

  auto imageConfigOptions = std::get_if<DockImageConfig>(&config);
  if (imageConfigOptions){
    std::function<void()> onClick =  [imageConfigOptions]() -> void {
      dockConfigApi.openImagePicker([imageConfigOptions](bool justClosed, std::string image) -> void {
        if (!justClosed){
          imageConfigOptions -> label = image;
          imageConfigOptions -> onImageSelect(image);
        }
      });
    };
    Props textboxProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = imageConfigOptions -> label },
        PropPair { .symbol = onclickSymbol, .value = onClick },
      }
    };
    auto textboxWithProps = withPropsCopy(textbox, textboxProps);
    return textboxWithProps; 
  }

  auto gameobjSelectorOptions = std::get_if<DockGameObjSelector>(&config);
  if (gameobjSelectorOptions){
    std::function<void()> onClick =  [gameobjSelectorOptions]() -> void {
      gameobjSelectorOptions -> label = std::nullopt;
      dockConfigApi.pickGameObj([gameobjSelectorOptions](objid id, std::string value) -> void {
        std::cout << "dock gameobjSelectorOptions onclick:  " << id << ", " << value << std::endl;
        gameobjSelectorOptions -> label = value;
        gameobjSelectorOptions -> onSelect(value);
      });
    };

    std::string value = gameobjSelectorOptions -> label.has_value() ? (std::string("target: ") + gameobjSelectorOptions -> label.value()) : std::string("none");
    Props textboxProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = value },
        PropPair { .symbol = onclickSymbol, .value = onClick },
      }
    };
    if (!gameobjSelectorOptions -> label.has_value()){
      textboxProps.props.push_back(PropPair {
        .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 1.f, 1.f),
      });
    }
    auto textboxWithProps = withPropsCopy(listItem, textboxProps);
    return textboxWithProps; 
  }

  auto dockGroupOptions = std::get_if<DockImageGroup>(&config);
  if (dockGroupOptions){
    std::vector<Component> elements;
    auto titleTextbox = withPropsCopy(listItem, Props {
      .props = {
        PropPair { .symbol = valueSymbol, .value = dockGroupOptions -> groupName },
        PropPair { .symbol = onclickSymbol, .value = dockGroupOptions -> onClick },
        PropPair { .symbol = fontsizeSymbol, .value = 0.02f },
        PropPair { .symbol = paddingSymbol, .value = 0.015f },
        PropPair { .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 0.f, 0.5f) },
      }
    });
    elements.push_back(titleTextbox);
    if (!dockGroupOptions -> collapse()){
      componentsForFields(dockGroupOptions -> configFields, elements);
    }
    return simpleVerticalLayout(elements, glm::vec2(0.f, 0.f), defaultAlignment, glm::vec4(0.f, 0.f, 0.f, 1.f), 0.01f);
  }

  auto dockScenegraphOptions = std::get_if<DockScenegraph>(&config);
  if (dockScenegraphOptions){
    return scenegraphContainer;
  }

  auto dockTextboxNumeric = std::get_if<DockTextboxNumeric>(&config);
  if (dockTextboxNumeric){
    Props textboxProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = std::string(dockTextboxNumeric -> label) + std::to_string(dockTextboxNumeric -> value) },
      }
    };
    auto textboxWithProps = withPropsCopy(textbox, textboxProps);
    return textboxWithProps;  
  }

  modassert(false, "dock component not yet implemented");
  return Component { };
}

void componentsForFields(std::vector<DockConfig>& configFields, std::vector<Component>& elements){
  for (auto &config : configFields){
    auto dockComponent = createDockComponent(config);
    elements.push_back(dockComponent);
  }
}

Component genericDockComponent {
  .draw = [](DrawingTools& drawTools, Props& props){
    auto dockType = strFromProp(props, dockTypeSymbol, "");
    auto dockConfig = dockConfigByName(dockType);
    modassert(dockConfig, std::string("dock config is null for: " + dockType));
    std::vector<Component> elements;
    componentsForFields(dockConfig -> configFields, elements);

    Layout layout {
      .tint = styles.secondaryColor,
      .showBackpanel = true,
      .borderColor = styles.highlightColor,
      .minwidth = 0.5f,
      .minheight = 0.f,
      .layoutType = LAYOUT_VERTICAL2, // LAYOUT_VERTICAL2,
      .layoutFlowHorizontal = UILayoutFlowNegative2, // L UILayoutFlowNone2,
      .layoutFlowVertical = UILayoutFlowNone2,
      .alignHorizontal = UILayoutFlowNegative2,
      .alignVertical = UILayoutFlowNone2,
      .spacing = 0.f,
      .minspacing = 0.f,
      .padding = 0.f,
      .children = elements,
    };

    Props listLayoutProps {
      .props = {
        { .symbol = layoutSymbol, .value = layout },
      },
    };

    auto boundingBox = withProps(layoutComponent, listLayoutProps).draw(drawTools, props);
    //drawDebugBoundingBox(drawTools, cameraBoundingBox, glm::vec4(1.f, 0.f, 0.f, 1.f));
    return boundingBox;
  },
};


Component dockComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    std::vector<Component> elements;
    auto dock = strFromProp(props, dockTypeSymbol, "");

    Props dockProps {
      .props = {
        PropPair { .symbol = dockTypeSymbol, .value = dock },
      }
    };
    elements.push_back(withProps(genericDockComponent, dockProps));

    // this interpolation (for animation) shouldn't be happening int he componnet here
    float xoffset = floatFromProp(props, xoffsetSymbol, 1.f);
    float xoffsetFrom = floatFromProp(props, xoffsetFromSymbol, xoffset);
    float interpAmount = floatFromProp(props, interpolationSymbol, 1.f);

    float xoffsetActual = (xoffset * interpAmount) + (xoffsetFrom * (1 - interpAmount));

    float yoffset = floatFromProp(props, yoffsetSymbol, 0.88f);

    Layout layout {
      .tint = styles.primaryColor,
      .showBackpanel = true,
      .borderColor = glm::vec4(1.f, 1.f, 1.f, 0.2f),
      .minwidth = 0.5f,
      .minheight = 1.f,
      .layoutType = LAYOUT_VERTICAL2,
      .layoutFlowHorizontal = UILayoutFlowNegative2,
      .layoutFlowVertical = UILayoutFlowNegative2,
      .alignHorizontal = UILayoutFlowNone2,
      .alignVertical = UILayoutFlowPositive2,
      .spacing = 0.f,
      .minspacing = 0.f,
      .padding = 0.f,
      .children = elements,
    };

    Props listLayoutProps {
      .props = {
        { .symbol = layoutSymbol, .value = layout },
        { .symbol = xoffsetSymbol, .value = xoffsetActual },
        { .symbol = yoffsetSymbol, .value = yoffset },
      },
    };
    return withProps(layoutComponent, listLayoutProps).draw(drawTools, props);
  },
};




