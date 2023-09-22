#include "./dock.h"

extern CustomApiBindings* gameapi;


enum DockFieldType {
  DOCK_BUTTON,
  DOCK_OPTION,
  DOCK_SLIDER,
  DOCK_CHECKBOX,
  DOCK_TEXTBOX,
};

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


extern DockConfigApi dockConfigApi;


int selectedIndex = 0;
bool checkboxChecked = true;
float minValuePercentage = 0.f;
float maxValuePercentage = 0.f;
float blurPercentage = 0.5f;



std::function<bool()> getIsCheckedWorld(std::string key, std::string attribute, std::string enabledValue, std::string disabledValue){
  return [key, attribute, enabledValue, disabledValue]() -> bool {
    auto value = dockConfigApi.getAttribute(key, attribute);
    auto valueStr = std::get_if<std::string>(&value);
    modassert(valueStr, "getIsCheckedWorld not strValue");
    return *valueStr == enabledValue;
  };
}

std::function<void(bool)> getOnCheckedWorld(std::string key, std::string attribute, std::string enabledValue, std::string disabledValue){
  return [key, attribute, enabledValue, disabledValue](bool checked) -> void {
    dockConfigApi.setAttribute(key, attribute, checked ? enabledValue : disabledValue);
  };
}

std::function<bool()> getIsCheckedGameobj(std::string key, std::string enabledValue, std::string disabledValue){
  return [key, enabledValue, disabledValue]() -> bool {
    auto attr = dockConfigApi.getObjAttr(key);
    if (!attr.has_value()){
      return false;
    }
    auto value = attr.value();
    auto valueStr = std::get_if<std::string>(&value);
    modassert(valueStr, "enabledValue not strValue");
    return *valueStr == enabledValue;
  };
}

std::function<void(bool)> getOnCheckedGameobj(std::string key, std::string enabledValue, std::string disabledValue){
  return [key, enabledValue, disabledValue](bool checked) -> void {
    dockConfigApi.setObjAttr(key, checked ? enabledValue : disabledValue);
  };
}

std::function<void(float)> getSetFloatGameobj(std::string key){
  return [key](float value) -> void {
    std::cout << "dock - set float gameobj: " << value << std::endl;
    dockConfigApi.setObjAttr(key, value);
  };
}

std::function<float()> getFloatGameobj(std::string key){
  return [key]() -> bool {
    auto attr = dockConfigApi.getObjAttr(key);
    if (!attr.has_value()){
      return 0.f;
    }
    auto value = attr.value();
    auto valueFloat = std::get_if<float>(&value);
    std::cout << "dock 2 - get float gameobj: " << *valueFloat << std::endl;
    modassert(valueFloat, "getFloatGameobj not float");
    return *valueFloat;
  };
}

/*
snap-rotate
        [
          "type" => "checkbox",
          "data" => [
            "key" => "enable physics", 
            "value" => [
              "binding" => "gameobj:physics",
              "binding-on" => "enabled",
              "binding-off" => "disabled",
            ],
          ],
        ],*/


/*
    "transform_types" => [
      "items" => [ 


        [
          "type" => "numeric",
          "data" => [
            "key" => "position", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "x", 
                "value" => [ 
                  "binding" => "gameobj:position", 
                  "binding-index" =>  0,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "y", 
                "value" => [ 
                  "binding" => "gameobj:position", 
                  "binding-index" =>  1,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "z", 
                "value" => [ 
                  "binding" => "gameobj:position", 
                  "binding-index" =>  2,
                  "type" => "number",
                ]
              ]
            ]
          ],
        ],


        [
          "type" => "numeric",
          "data" => [
            "key" => "scale", 
            "value" => [
              [ 
                "type" => "float", 
                "name" => "x", 
                "value" => [ 
                  "binding" => "gameobj:scale", 
                  "binding-index" =>  0,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "y", 
                "value" => [ 
                  "binding" => "gameobj:scale", 
                  "binding-index" =>  1,
                  "type" => "number",
                ]
              ],
              [ 
                "type" => "float", 
                "name" => "z", 
                "value" => [ 
                  "binding" => "gameobj:scale", 
                  "binding-index" =>  2,
                  "type" => "number",
                ]
              ]
            ]
          ],
        ],


      ],
    ],*/

struct OptionData {
  const char* optionName;

};
std::function<void(std::string& choice, int)> optionsOnClick(std::string key, std::string attribute, std::vector<AttributeValue> optionValueMapping){
  return [key, attribute, optionValueMapping](std::string& choice, int selectedIndex) -> void {
    dockConfigApi.setAttribute(key, attribute, optionValueMapping.at(selectedIndex));
  };
}
std::function<int()> optionsSelectedIndex(std::string key, std::string attribute, std::vector<AttributeValue> optionValueMapping){
  return [key, attribute, optionValueMapping]() -> int {
    auto attr = dockConfigApi.getAttribute(key, attribute);
    for (int i = 0; i < optionValueMapping.size(); i++){
      auto value = optionValueMapping.at(i);
      bool equal = aboutEqual(optionValueMapping.at(i), attr); 
      //std::cout << "comparing to: " << print(attr) << ", to " << print(value) << ", " << (equal ? "true" : "false") << std::endl;
      if (equal){
        return i;
      }
    }
    modassert(false, "options selected index - invalid");
    return 0;
  };
}

std::function<void(std::string& choice, int)> optionsOnClickObj(std::string attribute, std::vector<AttributeValue> optionValueMapping){
  return [attribute, optionValueMapping](std::string& choice, int selectedIndex) -> void {
    dockConfigApi.setObjAttr(attribute, optionValueMapping.at(selectedIndex));
  };
}
std::function<int()> optionsSelectedIndexObj(std::string attribute, std::vector<AttributeValue> optionValueMapping){
  return [attribute, optionValueMapping]() -> int {
    auto attr = dockConfigApi.getObjAttr(attribute);
    if (!attr.has_value()){
      return false;
    }

    for (int i = 0; i < optionValueMapping.size(); i++){
      auto value = optionValueMapping.at(i);
      bool equal = aboutEqual(optionValueMapping.at(i), attr.value()); 
      //std::cout << "comparing to: " << print(attr) << ", to " << print(value) << ", " << (equal ? "true" : "false") << std::endl;
      if (equal){
        return i;
      }
    }
    return -1;
  };
}


bool collapseTestGroup = true;
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
    .title = "Textures",
    .configFields = {
      DockTextboxNumeric {
        .label = "tiling x",
        .value = 10.f,
        // gameobj:texturetiling
      },
      DockTextboxNumeric {
        .label = "tiling y",
        .value = 20.f,
      },
      DockTextboxNumeric {
        .label = "texturesize x",
        .value = 20.f,
        // gameobj:texturetiling
      },
      DockTextboxNumeric {
        .label = "texturesize y",
        .value = 20.f,
        // gameobj:texturetiling
      },
      DockTextboxNumeric {
        .label = "textureoffset x",
        .value = 20.f,
        // gameobj:textureoffset
      },
      DockTextboxNumeric {
        .label = "textureoffset y",
        .value = 20.f,
        // gameobj:textureoffset
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
        .onClick = []() -> void { collapseTestGroup = !collapseTestGroup; },
        .collapse = []() -> bool { return collapseTestGroup;  },
        .configFields = {
          DockGameObjSelector {
            .label = std::nullopt,
          },
          DockCheckboxConfig {
            .label = "toggle dof",
            .isChecked = getIsCheckedWorld("skybox", "enable", "true", "false"),
            .onChecked = getOnCheckedWorld("skybox", "enable", "true", "false"),
          },
          DockSliderConfig {
            .label = "blur min",
            .min = 0.f,
            .max = 1.f,
            .percentage = getFloatGameobj("minblur"),
            .onSlide =  getSetFloatGameobj("minblur"),
          },
          DockSliderConfig {
            .label = "blur max",
            .min = 0.f,
            .max = 1.f,
            .percentage = []() -> float { return maxValuePercentage; },
            .onSlide = [](float value) -> void {
              std::cout << "dock slider on slide max: " << value << std::endl;
              maxValuePercentage = value;
            },
          },
          DockSliderConfig {
            .label = "blur amount",
            .min = 0.f,
            .max = 1.f,
            .percentage = []() -> float { return blurPercentage; },
            .onSlide = [](float value) -> void {
              std::cout << "dock slider on slide blur: " << value << std::endl;
              blurPercentage = value;
            },
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
        .onClick = [](std::string& choice, int) -> void {
          std::cout << "dock mock toggle light mode: " << choice << std::endl;
          if (choice == "point"){
            selectedIndex = 0;
          }else if (choice == "spotlight"){
            selectedIndex = 1;
          }else if (choice == "directional"){
            selectedIndex = 2;
          }
        },
        .getSelectedIndex = []() -> int { 
          return selectedIndex; 
        },
      },
      DockTextboxConfig {
        .text = []() -> std::string {
          return "some text here"; 
        },
        .onEdit = [](std::string value) -> void {

        }
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
      DockCheckboxConfig {
        .label = "Show Cameras",
        .isChecked = getIsCheckedWorld("editor", "debug", "true", "false"),
        .onChecked = getOnCheckedWorld("editor", "debug", "true", "false"),
      },
      DockCheckboxConfig {
        .label = "Show Lights",
        .isChecked = getIsCheckedWorld("editor", "debug", "true", "false"),
        .onChecked = getOnCheckedWorld("editor", "debug", "true", "false"),
      },
      DockCheckboxConfig {
        .label = "Show Sound",
        .isChecked = getIsCheckedWorld("editor", "debug", "true", "false"),
        .onChecked = getOnCheckedWorld("editor", "debug", "true", "false"),
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
        .onClick = optionsOnClick("tools", "manipulator-mode", { "translate", "scale", "rotate" }),
        .getSelectedIndex = optionsSelectedIndex("tools", "manipulator-mode", { "translate", "scale", "rotate" }),
      },
      DockButtonConfig {
        .buttonText = "copy",
        .onClick = []() -> void {},
      },
      DockButtonConfig {
        .buttonText = "paste",
        .onClick = []() -> void {},
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
  //return &configurations.at(0);
}

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
    Props textboxProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = textboxOptions -> text() },
        PropPair { .symbol = editableSymbol, .value = true },
      }
    };
    auto textboxWithProps = withPropsCopy(textbox, textboxProps);
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
      .tint = glm::vec4(0.f, 0.f, 1.f, 0.2f),
      .showBackpanel = false,
      .borderColor = glm::vec4(0.f, 0.f, 0.f, 1.f),
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
      .tint = glm::vec4(0.f, 0.f, 0.f, 0.5f),
      .showBackpanel = false,
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




