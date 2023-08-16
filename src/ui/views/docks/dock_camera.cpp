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


std::optional<std::function<void()>> nullClick = []() -> void {

};

Props cameraOptions(){
  std::vector<ListComponentData> levels;
  levels.push_back(ListComponentData {
  	.name = "one",
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
    .name = "four",
    .onClick = nullClick,
  });
  
  Props levelProps {
    .props = {
      PropPair { .symbol = listItemsSymbol, .value = levels },
      PropPair { .symbol = xoffsetSymbol,   .value = -0.81f },
      PropPair { .symbol = yoffsetSymbol,   .value = 0.98f },
      PropPair { .symbol = tintSymbol,      .value = glm::vec4(0.f, 0.f, 0.f, 1.f) },
      PropPair { .symbol = horizontalSymbol,   .value = true },
      PropPair { .symbol = paddingSymbol,      .value = 0.02f },
    },
  };
  return levelProps;
}

Component dockCameraComponent {
  .draw = [](DrawingTools& drawTools, Props& props){	
  	Props defaultProps { .props = {} };
  	return withPropsCopy(listComponent,  cameraOptions()).draw(drawTools, props);
  },
};
