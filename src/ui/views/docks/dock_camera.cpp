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



Props cameraOptions(){
  std::vector<ListComponentData> levels;
  levels.push_back(ListComponentData {
  	.name = "docker camera item 1",
  	.onClick = nullClick,
  });
  levels.push_back(ListComponentData {
    .name = "two",
    .onClick = nullClick,
  });
  levels.push_back(ListComponentData {
    .name = "three",
    .onClick = nullClick,
  });
  levels.push_back(ListComponentData {
    .name = "docker camera last item",
    .onClick = nullClick,
  });
  
  Props levelProps {
    .props = {
      //PropPair { .symbol = listItemsSymbol, .value = levels },
      //PropPair { .symbol = xoffsetSymbol,   .value = -0.81f },
      //PropPair { .symbol = yoffsetSymbol,   .value = 0.98f },
      //PropPair { .symbol = tintSymbol,      .value = glm::vec4(0.f, 0.f, 0.f, 1.f) },
      //PropPair { .symbol = horizontalSymbol,   .value = true },
      //PropPair { .symbol = paddingSymbol,      .value = 0.02f },
    },
  };
  return levelProps;
}

int value = 2;
std::function<void()> defaultOnClick = [value]() -> void {
  std::cout << "hello world on click from button" << value << std::endl;
};

Component dockCameraComponent {
  .draw = [](DrawingTools& drawTools, Props& props){
    std::vector<Component> elements;

    Props buttonProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = std::string("create camera") },
        PropPair { .symbol = onclickSymbol, .value = defaultOnClick }, 
      }
    };
    elements.push_back(withProps(button, buttonProps));

    Options defaultOptions {
      .options = {
        Option {
          .name = "opt1",
          .onClick = nullClick,
        },
        Option {
          .name = "opt2",
          .onClick = nullClick,
        }
      },
    };
    Props optionsProps {
      .props = {
        PropPair { .symbol = optionsSymbol, .value = defaultOptions },
      }
    };
    elements.push_back(withProps(options, optionsProps));
    elements.push_back(withProps(options, optionsProps));


    Layout layout {
      .tint = glm::vec4(0.f, 0.f, 0.f, 0.8f),
      .showBackpanel = false,
      .borderColor = glm::vec4(0.f, 1.f, 0.f, 0.8f),
      .minwidth = 0.f,
      .minheight = 0.f,
      .layoutType = LAYOUT_VERTICAL2, // LAYOUT_VERTICAL2,
      .layoutFlowHorizontal = UILayoutFlowNone2, // L UILayoutFlowNone2,
      .layoutFlowVertical = UILayoutFlowNone2,
      .alignHorizontal = UILayoutFlowNone2,
      .alignVertical = UILayoutFlowNone2,
      .spacing = 0.f,
      .minspacing = 0.2f,
      .padding = 0.f,
      .children = elements,
    };

    Props listLayoutProps {
      .props = {
        { .symbol = layoutSymbol, .value = layout },
      },
    };
    return withProps(layoutComponent, listLayoutProps).draw(drawTools, props);

  },
};
