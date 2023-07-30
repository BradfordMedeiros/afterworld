#include "./menus.h"

extern CustomApiBindings* gameapi;

int mappingId = 999995;
std::vector<NestedListItem> nestedListTest = {
  NestedListItem {
    .item = ImListItem {
      .value = "file",
      .onClick = std::nullopt,
      .mappingId = mappingId++,
    },
    .items = {
      NestedListItem {
        .item = ImListItem {
          .value = "fullscreen",
          .onClick = []() -> void {
            auto isFullscreen = getStrWorldState("rendering", "fullscreen").value() == "true";
            gameapi -> setWorldState({ 
              ObjectValue {
                .object = "rendering",
                .attribute = "fullscreen",
                .value = isFullscreen ? "false" : "true",
              }
            });
          },
          .mappingId = mappingId++,
        },
        .items = {},
      },
      NestedListItem {
        .item = ImListItem {
          .value = "layout",
          .onClick = std::nullopt, // maybe this should send a request to core engine so can shutdown properly }
          .mappingId = mappingId++,
        },
        .items = {
          NestedListItem {
            .item = ImListItem {
              .value = "main",
              .onClick = std::nullopt, // maybe this should send a request to core engine so can shutdown properly }
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "gun editor",
              .onClick = std::nullopt, // maybe this should send a request to core engine so can shutdown properly }
              .mappingId = mappingId++,
            },
            .items = {},
          },
        },
      },
      NestedListItem {
        .item = ImListItem {
          .value = "quit",
          .onClick = []() -> void { exit(0); }, // maybe this should send a request to core engine so can shutdown properly }
          .mappingId = mappingId++,
        },
        .items = {},
      },
    },
  },
  NestedListItem {
    .item = ImListItem {
      .value = "play/pause",
      .onClick = []() -> void { 
        auto isPaused = toggleWorldStateBoolStr("world", "paused");
        nestedListTest.at(1).item.value = (isPaused ? "play" : "pause");
      },
      .mappingId = mappingId++,
    },
    .items = {},
  },
  NestedListItem {
    .item = ImListItem {
      .value = "render",
      .onClick = std::nullopt,
      .mappingId = mappingId++,
    },
    .items = {
      NestedListItem {
        .item = ImListItem {
          .value = "lighting",
          .onClick = std::nullopt,
          .mappingId = mappingId++,
        },
        .items = {
          NestedListItem {
            .item = ImListItem {
              .value = "diffuse",
              .onClick = getToggleWorldStateBoolStr("diffuse", "enabled"),
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "specular",
              .onClick = getToggleWorldStateBoolStr("specular", "enabled"),
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "bloom",
              .onClick = getToggleWorldStateBoolStr("bloom", "enabled"),
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "attenuation",
              .onClick = getToggleWorldStateBoolStr("attenuation", "enabled"),
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "shadows",
              .onClick = getToggleWorldStateBoolStr("shadows", "enabled"),
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "pbr",
              .onClick = getToggleWorldStateBoolStr("pbr", "enabled"),
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "exposure",
              .onClick = getToggleWorldStateBoolStr("exposure", "enabled"),
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "gamma",
              .onClick = getToggleWorldStateBoolStr("gamma", "enabled"),
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "dof",
              .onClick = getToggleWorldStateBoolStr("dof", "state", "enabled", "disabled"),
              .mappingId = mappingId++,
            },
            .items = {},
          }
        },
      },
      NestedListItem {
        .item = ImListItem {
          .value = "skybox",
          .onClick = getToggleWorldStateBoolStr("skybox", "enable"),
          .mappingId = mappingId++,
        },
        .items = {},
      },
      NestedListItem {
        .item = ImListItem {
          .value = "culling",
          .onClick = getToggleWorldStateBoolStr("rendering", "cull", "enabled", "disabled"),
          .mappingId = mappingId++,
        },
        .items = {},
      },
      NestedListItem {
        .item = ImListItem {
          .value = "fog",
          .onClick = std::nullopt,
          .mappingId = mappingId++,
        },
        .items = {
          NestedListItem {
            .item = ImListItem {
              .value = "enabled",
              .onClick = getToggleWorldStateBoolStr("fog", "enabled"),
              .mappingId = mappingId++,
            },
            .items = {},
          },          
        },
      },
    },
  },
  NestedListItem {
    .item = ImListItem {
      .value = "editor",
      .onClick = std::nullopt,
      .mappingId = mappingId++,
    },
    .items = {
      NestedListItem {
        .item = ImListItem {
          .value = "grid",
          .onClick = getToggleWorldStateBoolStr("editor", "showgrid"),
          .mappingId = mappingId++,
        },
        .items = {},
      },
      NestedListItem {
        .item = ImListItem {
          .value = "translate",
          .onClick = getToggleWorldStateSetStr("tools", "manipulator-mode", "translate"),
          .mappingId = mappingId++,
        },
        .items = {
          NestedListItem {
            .item = ImListItem {
              .value = "position-relative",
              .onClick = getToggleWorldStateBoolStr("tools", "position-relative"),
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "position-mirror",
              .onClick = getToggleWorldStateBoolStr("tools", "position-mirror"),
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "preserve-scale",
              .onClick = getToggleWorldStateBoolStr("tools", "preserve-scale"),
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "snap 0",
              .onClick = getToggleWorldStateSetFloat("editor", "snaptranslate-index", 0),
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "snap 1",
              .onClick = getToggleWorldStateSetFloat("editor", "snaptranslate-index", 5),
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "snap 10",
              .onClick = getToggleWorldStateSetStr("tools", "manipulator-mode", "translate"),
              .mappingId = mappingId++,
            },
            .items = {},
          },
        },
      },
      NestedListItem {
        .item = ImListItem {
          .value = "scale",
          .onClick = getToggleWorldStateSetStr("tools", "manipulator-mode", "scale"),
          .mappingId = mappingId++,
        },
        .items = {},
      },
      NestedListItem {
        .item = ImListItem {
          .value = "rotate",
          .onClick = getToggleWorldStateSetStr("tools", "manipulator-mode", "rotate"),
          .mappingId = mappingId++,
        },
        .items = {},
      },
      NestedListItem {
        .item = ImListItem {
          .value = "create obj",
          .onClick = std::nullopt,
          .mappingId = mappingId++,
        },
        .items = {
          NestedListItem {
            .item = ImListItem {
              .value = "camera",
              .onClick = std::nullopt,
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "light",
              .onClick = std::nullopt,
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "sound",
              .onClick = std::nullopt,
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "heightmap",
              .onClick = std::nullopt,
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "portal",
              .onClick = std::nullopt,
              .mappingId = mappingId++,
            },
            .items = {},
          },
        },
      },
    },
  },
  NestedListItem {
    .item = ImListItem {
      .value = "debug",
      .onClick = []() -> void { }, // maybe this should send a request to core engine so can shutdown properly }
      .mappingId = mappingId++,
    },
    .items = {
      NestedListItem {
        .item = ImListItem {
          .value = "show debug",
          .onClick = getToggleWorldStateBoolStr("editor", "debug"),
          .mappingId = mappingId++,
        },
        .items = {},
      },
      NestedListItem {
        .item = ImListItem {
          .value = "screen grid",
          .onClick = []() -> void { 
            getGlobalState().showScreenspaceGrid = !getGlobalState().showScreenspaceGrid; 
          },
          .mappingId = mappingId++,
        },
        .items = {},
      },
      NestedListItem {
        .item = ImListItem {
          .value = "mute sound",
          .onClick = getToggleWorldStateBoolStr("sound", "mute"),
          .mappingId = mappingId++,
        },
        .items = {},
      },
    },
  },
  NestedListItem {
    .item = ImListItem {
      .value = "scene",
      .onClick = std::nullopt, 
      .mappingId = mappingId++,
    },
    .items = {
      NestedListItem {
        .item = ImListItem {
          .value = "new scene",
          .onClick = notYetImplementedAlert,
          .mappingId = mappingId++,
        },
        .items = {},
      },
      NestedListItem {
        .item = ImListItem {
          .value = "save scene",
          .onClick = notYetImplementedAlert,
          .mappingId = mappingId++,
        },
        .items = {},
      },
      NestedListItem {
        .item = ImListItem {
          .value = "reset scene",
          .onClick = notYetImplementedAlert, // maybe this should send a request to core engine so can shutdown properly }
          .mappingId = mappingId++,
        },
        .items = {},
      },
    },
  },
  NestedListItem {
    .item = ImListItem {
      .value = "misc fun",
      .onClick = std::nullopt, 
      .mappingId = mappingId++,
    },
    .items = {
      NestedListItem {
        .item = ImListItem {
          .value = "worldstate",
          .onClick = []() -> void { 
          },
          .mappingId = mappingId++,
        },
        .items = {
          NestedListItem {
            .item = ImListItem {
              .value = "save",
              .onClick = []() -> void { 
              },
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "load 1",
              .onClick = []() -> void { 
              },
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "load 2",
              .onClick = []() -> void { 
              },
              .mappingId = mappingId++,
            },
            .items = {},
          },
        },
      },
      NestedListItem {
        .item = ImListItem {
          .value = "background",
          .onClick = std::nullopt,
          .mappingId = mappingId++,
        },
        .items = {
          NestedListItem {
            .item = ImListItem {
              .value = "dark-trippy",
              .onClick = []() -> void { 
                gameapi -> sendNotifyMessage("menu-background", std::string("/home/brad/Desktop/test3.png"));
              },
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "dark-trippy2",
              .onClick = []() -> void { 
                gameapi -> sendNotifyMessage("menu-background", std::string("/home/brad/Desktop/test4.png"));
              },
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "cybertek",
              .onClick = []() -> void { 
                gameapi -> sendNotifyMessage("menu-background", std::string("/home/brad/Desktop/test.png"));
              },
              .mappingId = mappingId++,
            },
            .items = {},
          },
        },
      },
      NestedListItem {
        .item = ImListItem {
          .value = "hotkeys",
          .onClick = std::nullopt, 
          .mappingId = mappingId++,
        },
        .items = {
          NestedListItem {
            .item = ImListItem {
              .value = "engine-dev",
              .onClick = notYetImplementedAlert,
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "editor-keys",
              .onClick = notYetImplementedAlert,
              .mappingId = mappingId++,
            },
            .items = {},
          },
        },
      },
      NestedListItem {
        .item = ImListItem {
          .value = "font",
          .onClick = std::nullopt,
          .mappingId = mappingId++,
        },
        .items = {
          NestedListItem {
            .item = ImListItem {
              .value = "Walby-Regular.ttf",
              .onClick = []() -> void { 
              },
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "dpquak e.ttf",
              .onClick = []() -> void { 
              },
              .mappingId = mappingId++,
            },
            .items = {},
          },
        },
      },
    },
  },
};

Component nestedListTestComponent = createNestedList(nestedListTest);


std::vector<ImListItem> createPauseMenu(std::function<void()> resume, std::function<void()> goToMainMenu){
  int pauseMappingIds = 999999;
  std::vector<ImListItem> listItems;
  listItems.push_back(ImListItem {
    .value = "Resume",
    .onClick = resume,
    .mappingId = pauseMappingIds++,
  });
  listItems.push_back(ImListItem {
    .value = "Main Menu",
    .onClick = goToMainMenu,
    .mappingId = pauseMappingIds++,
  });
  return listItems;
}




ImageList defaultImages {
  .mappingId = 858584,
  .images = {
    "./res/textures/wood.jpg",
    "./res/textures/grass.jpg",
    "./res/textures/brickwall.jpg",
    "./res/textures/grid.png",
    "./res/textures/grass.jpg",
    "./res/textures/grid.png",
    "./res/textures/wood.jpg",
    "./res/textures/brickwall.jpg",
  },
};

Component imageListTest = createImageList(defaultImages);

Slider exampleSlider {
  .min  = 0.f,
  .max = 10.f,
  .percentage = 0.f,
  .mappingId = 34545,
  .update = false,
};
Component sliderSelector = createSlider(exampleSlider);


int mappingIdRadio = 95000;
RadioButtonContainer radioButtonContainer {
  .selectedRadioButtonIndex = 0,
  .radioButtons = { 
    RadioButton {
      .selected = false,
      .hovered = false,
      .onClick = []() -> void {
        std::cout << "on click button 0" << std::endl;
      },
      .mappingId = mappingIdRadio++,
    },
    RadioButton {
      .selected = false,
      .hovered = false,
      .onClick = std::nullopt,
      .mappingId = mappingIdRadio++,
    },
    RadioButton {
      .selected = false,
      .hovered = false,
      .onClick = []() -> void {
        std::cout << "on click button 2" << std::endl;
      },
      .mappingId = mappingIdRadio++,
    }
  }
};
Component radioButtonSelector = createRadioButtonComponent(radioButtonContainer);

Component createListItem(std::string value){
  ImListItem menuItem {
    .value = value,
    .onClick = []() -> void {
    },
    .mappingId = 0,
  };
  auto component = Component {
    .draw = [menuItem](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
        auto box = drawImMenuListItem(drawTools, menuItem, props.mappingId,  props.style, props.additionalYOffset);
        //auto yoffset = getProp<int>(props, symbolForName("yoffset"));
        drawDebugBoundingBox(drawTools, box);
        return box;
    },
    .imMouseSelect = [menuItem](std::optional<objid> mappingIdSelected) -> void {
      //if (mappingIdSelected.has_value() && mappingIdSelected.value() == menuItem.mappingId.value()){
      //  if (menuItem.onClick.has_value()){
      //    menuItem.onClick.value()();
      //  }
      //}
    },
  };
  return component;
}

Layout testLayout {
  .tint = glm::vec4(0.f, 0.f, 1.f, .6f),
  .showBackpanel = true,
  .borderColor = glm::vec4(0.f, 1.f, 0.f, 0.6f),
  .minwidth = 1.5f,
  .minheight = 0.4f,
  .layoutType = LAYOUT_HORIZONTAL2, //LAYOUT_VERTICAL2,
  .layoutFlowHorizontal = UILayoutFlowPositive2,
  .layoutFlowVertical = UILayoutFlowPositive2,

  //.spacing = 0.1f,
  .spacing = 0.f,
  //.minspacing = 0.4f,
  .minspacing = 0.f,
  //.margin = 0.02f,
  .margin = 0.f,
  .children = {
    createListItem("one"),
    createListItem("two"),
    createListItem("three"),
  },
};

Component testLayoutComponent = createLayoutComponent(testLayout);