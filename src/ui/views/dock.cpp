#include "./dock.h"

extern CustomApiBindings* gameapi;


enum DockFieldType {
  DOCK_BUTTON,
  DOCK_OPTION,
  DOCK_SLIDER,
  DOCK_CHECKBOX,
  DOCK_TEXTBOX,
};

struct DockButtonConfig {
  const char* buttonText;
  std::function<void()> onClick;
};
struct DockOptionConfig {
  std::vector<const char*> options;
  std::function<void(std::string&)> onClick;
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


struct DockImageGroup;

typedef std::variant<DockButtonConfig, DockOptionConfig, DockSliderConfig, DockCheckboxConfig, DockTextboxConfig, DockFileConfig, DockImageConfig, DockImageGroup> DockConfig;

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
    dockConfigApi.setAttribute("skybox", "enable", checked ? enabledValue : disabledValue);
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
    .title = "Cameras",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Create Camera",
        .onClick = []() -> void { dockConfigApi.createCamera(); },
      },
      DockOptionConfig {
        .options = { "enable dof", "disable dof" },
        .onClick = [](std::string& choice) -> void {
          std::cout << "dock mock enable dof: " << choice << std::endl;
          if (choice == "enable dof"){
            selectedIndex = 0;
          }else if (choice == "disable dof"){
            selectedIndex = 1;
          }
        },
        .getSelectedIndex = []() -> int { return selectedIndex; },
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
        .onClick = [](std::string& choice) -> void {
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
      }
    },
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
    for (auto &option : dockOptions -> options){
      auto onClick = dockOptions -> onClick;
      optionsData.push_back(Option {
        .name = option,
        .onClick = [onClick, option]() -> void {
          std::string optionStr(option);
          onClick(optionStr);
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
      });
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

  auto dockGroupOptions = std::get_if<DockImageGroup>(&config);
  if (dockGroupOptions){
    std::vector<Component> elements;
    auto titleTextbox = withPropsCopy(listItem, Props {
      .props = {
        PropPair { .symbol = valueSymbol, .value = dockGroupOptions -> groupName },
        PropPair { .symbol = onclickSymbol, .value = dockGroupOptions -> onClick },
        PropPair { .symbol = fontsizeSymbol, .value = 0.02f },
        PropPair { .symbol = paddingSymbol, .value = 0.015f },
      }
    });
    elements.push_back(titleTextbox);
    if (!dockGroupOptions -> collapse()){
      componentsForFields(dockGroupOptions -> configFields, elements);
    }
    return simpleVerticalLayout(elements, glm::vec2(0.f, 0.f), defaultAlignment, glm::vec4(0.f, 0.f, 1.f, 1.f), 0.01f);
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
      .showBackpanel = true,
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



