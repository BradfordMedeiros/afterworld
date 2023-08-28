#include "./dock_camera.h"


/*

<button>
<checkbox - toggle dof> 
<numeric [ { slider, slider, number} ]
<label selector> 

    [
          "type" => "label",
          "data" => [
            "key" => "Create Camera",
            "readonly" => true,
            "value" => [
              "binding" => "false-binding-0",
            ],
            "action" => "create-camera",
            "tint" => "1 1 0 1",
          ],
        ],
        [
          "type" => "checkbox",
          "data" => [
            "key" => "depth of field", 
            "value" => [
              "binding" => "gameobj:dof",  
              "binding-on" => "enabled",
              "binding-off" => "disabled",
            ],
          ],
        ],
        [
          "type" => "numeric",
          "data" => [
            "key" => "blur distances for depth of field effect", 
            "value" => [
              [ 
                "type" => "slider", 
                "name" => "min blur", 
                "value" => [ 
                  "binding" => "gameobj:minblur", 
                  "min" => -100,  // what should min and max really be?d
                  "max" => 100,
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "maxblur", 
                "value" => [ 
                  "binding" => "gameobj:maxblur", 
                  "min" => -100,
                  "max" => 100,
                ]
              ],
              [ 
                "type" => "slider", 
                "name" => "amount", 
                "value" => [ 
                  "binding" => "gameobj:bluramount", 
                  "min" => 0,
                  "max" => 100,
                ]
              ]
            ]
          ],
        ],
        [
          "type" => "label",
          "data" => [
            "key" => "Depth of Field Target",
            "value" => [
              "valueFromSelection" => true,
              "binding" => "gameobj:target",
            ]
          ],
        ],
      ],
    ],
    */

enum DockFieldType {
  DOCK_BUTTON,
  DOCK_OPTION,
  DOCK_SLIDER,
  DOCK_CHECKBOX,
  DOCK_TEXTBOX,
};

struct DockButtonConfig {
  const char* buttonText;
};
struct DockOptionConfig {
  std::vector<const char*> options;
};
struct DockSliderConfig {

};
struct DockCheckboxConfig {

};
struct DockTextboxConfig {

};
typedef std::variant<DockButtonConfig, DockOptionConfig, DockSliderConfig> DockConfig;

struct DockConfiguration {
  std::string title;
  std::vector<DockConfig> configFields;
};


std::vector<DockConfiguration> configurations {
  DockConfiguration {
    .title = "Cameras",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Create Camera Yo",
      },
      DockOptionConfig {
        .options = { "one", "two" },
      },
      DockSliderConfig {

      },
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
}

int value = 2;
std::function<void()> defaultOnClick = [value]() -> void {
  std::cout << "hello world on click from button" << value << std::endl;
};

Component dockCameraComponent {
  .draw = [](DrawingTools& drawTools, Props& props){
    std::vector<Component> elements;

    auto dockConfig = dockConfigByName("Cameras");
    modassert(dockConfig, "dock config is null");
    for (auto &config : dockConfig -> configFields){
      auto dockButton = std::get_if<DockButtonConfig>(&config);
      if (dockButton){
        Props buttonProps {
          .props = {
            PropPair { .symbol = valueSymbol, .value = std::string(dockButton -> buttonText) },
            PropPair { .symbol = onclickSymbol, .value = defaultOnClick }, 
          }
        };
        elements.push_back(withPropsCopy(button, buttonProps));  
        continue;
      }
    }



    Options defaultOptions {
      .options = {
        Option {
          .name = "opt1",
          .onClick = nullClick,
        },
      },
    };
    Props optionsProps {
      .props = {
        PropPair { .symbol = optionsSymbol, .value = defaultOptions },
      }
    };
    //elements.push_back(withProps(options, optionsProps));

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

    auto sliderWithProps = withProps(slider, sliderProps); 
    //elements.push_back(sliderWithProps);

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

    std::cout << "dock camera - start draw" << std::endl;
    auto cameraBoundingBox = withProps(layoutComponent, listLayoutProps).draw(drawTools, props);
    std::cout << "dock camera - end draw" << std::endl << std::endl;
    //drawDebugBoundingBox(drawTools, cameraBoundingBox, glm::vec4(1.f, 0.f, 0.f, 1.f));
    return cameraBoundingBox;
  },
};
