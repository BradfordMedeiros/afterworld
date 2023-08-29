#include "./dock.h"

const int dockTypeSymbol = getSymbol("dock-type");

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
};
struct DockSliderConfig {

};
struct DockCheckboxConfig {
  std::string label;
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
    std::cout << "dock config mock make camera" << std::endl;
  },
  .createLight = []() -> void {
    std::cout << "dock config mock make light" << std::endl;
  },
};


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
        .buttonText = "Create Camera Yo",
        .onClick = dockConfig.createCamera,
      },
      DockOptionConfig {
        .options = { "enable dof", "disable dof" },
      },
      DockSliderConfig {

      },
      DockCheckboxConfig {
        .label = "toggle dof",
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
  return &configurations.at(0);
}


int value = 2;
std::function<void()> defaultOnClick = [value]() -> void {
  std::cout << "hello world on click from button" << value << std::endl;
};

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
          optionsData.push_back(Option {
            .name = option,
            .onClick = nullClick,
          });
        }
        Options defaultOptions {
          .options = optionsData,
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
          .update = false,
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
            PropPair { .symbol = checkedSymbol, .value = false },
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

    auto strValue = strFromProp(props, titleSymbol, "Dock");
    std::function<void()> onClick = []() -> void { std::cout << "on click" << std::endl; };
    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = std::string("Dock") },
        PropPair { .symbol = onclickSymbol, .value = onClick },
      },
    };
    auto listItemWithProps = withPropsCopy(listItem, listItemProps);

    elements.push_back(listItemWithProps);

    Props dockProps {
      .props = {
        PropPair { .symbol = dockTypeSymbol, .value = strValue },
      }
    };
    elements.push_back(withProps(genericDockComponent, dockProps));


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



