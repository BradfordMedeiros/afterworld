#include "./dock.h"

enum DockFieldType {
  DOCK_BUTTON,
  DOCK_OPTION,
  DOCK_SLIDER,
  DOCK_CHECKBOX,
  DOCK_TEXTBOX,
};

struct DockButtonConfig {
  const char* buttonText;
  std::optional<std::function<void()>> onClick;
};
struct DockOptionConfig {
  std::vector<const char*> options;
  std::function<void(std::string&)> onClick;
  std::function<int(void)> getSelectedIndex;
};
struct DockSliderConfig {

};
struct DockCheckboxConfig {
  std::string label;
  std::function<bool()> isChecked;
  std::function<void(bool)> onChecked;
};
struct DockTextboxConfig {
  std::string text;
};
typedef std::variant<DockButtonConfig, DockOptionConfig, DockSliderConfig, DockCheckboxConfig, DockTextboxConfig> DockConfig;

struct DockConfiguration {
  std::string title;
  std::vector<DockConfig> configFields;
};

struct DockConfigApi {
  std::function<void()> createCamera;
  std::function<void()> createLight;
};

DockConfigApi dockConfig {
  .createCamera = []() -> void {
    modlog("dock", "mock make camera");
  },
  .createLight = []() -> void {
    modlog("dock", "mock make light");
  },
};


int selectedIndex = 0;
bool checkboxChecked = true;
std::vector<DockConfiguration> configurations {
  DockConfiguration {
    .title = "",
    .configFields = {
      DockButtonConfig {
        .buttonText = "no panel available",
        .onClick = std::nullopt,
      },
    },
  },
  DockConfiguration {
    .title = "Cameras",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Create Camera",
        .onClick = dockConfig.createCamera,
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
      DockSliderConfig {

      },
      DockCheckboxConfig {
        .label = "toggle dof",
        .isChecked = []() -> bool { return checkboxChecked; },
        .onChecked = [](bool checked) -> void {
          std::cout << "dock toggledof: " << checked << std::endl;
          checkboxChecked = checked;
        },
      },
    },
  },
  DockConfiguration {
    .title = "Lights",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Create Light",
        .onClick = dockConfig.createLight,
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
        .text = "some text here",
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



Component genericDockComponent {
  .draw = [](DrawingTools& drawTools, Props& props){
    std::vector<Component> elements;
    auto dockType = strFromProp(props, dockTypeSymbol, "");
    auto dockConfig = dockConfigByName(dockType);
    modassert(dockConfig, std::string("dock config is null for: " + dockType));
    for (auto &config : dockConfig -> configFields){
      auto dockButton = std::get_if<DockButtonConfig>(&config);
      if (dockButton){
        Props buttonProps {
          .props = {
            PropPair { .symbol = valueSymbol, .value = std::string(dockButton -> buttonText) },
          }
        };
        if (dockButton -> onClick.has_value()){
          buttonProps.props.push_back(PropPair { .symbol = onclickSymbol, .value =  dockButton -> onClick.value() });
        }
        elements.push_back(withPropsCopy(button, buttonProps));  
        continue;
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
        elements.push_back(withPropsCopy(options, optionsProps));
        continue;
      }

      auto sliderOptions = std::get_if<DockSliderConfig>(&config);
      if (sliderOptions){
        Slider sliderData {
          .min = 0.f,
          .max = 10.f,
          .percentage = 0.5f,
        };
        Props sliderProps {
          .props = {
            PropPair { .symbol = sliderSymbol, .value = sliderData },
          },
        };
    
        auto sliderWithProps = withPropsCopy(slider, sliderProps); 
        elements.push_back(sliderWithProps);
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
        elements.push_back(checkboxWithProps);
      }

      auto textboxOptions = std::get_if<DockTextboxConfig>(&config);
      if (textboxOptions){
        Props textboxProps {
          .props = {
            PropPair { .symbol = valueSymbol, .value = textboxOptions -> text },
            PropPair { .symbol = editableSymbol, .value = true },
          }
        };
        auto textboxWithProps = withPropsCopy(textbox, textboxProps);
        elements.push_back(textboxWithProps);
      }
    }

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



