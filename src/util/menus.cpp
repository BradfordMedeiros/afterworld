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

std::vector<RadioButton> createRadioButtons(){
  int radioButtonMappingId = 342089;
  auto manipulatorMode = getStrWorldState("tools", "manipulator-mode").value();
  std::vector<RadioButton> radioButtonTest = {
    RadioButton { 
      .selected = manipulatorMode == "translate", 
      .onClick = getToggleWorldStateSetStr("tools", "manipulator-mode", "translate"),
      .mappingId = radioButtonMappingId++ 
    },
    RadioButton { 
      .selected = manipulatorMode == "scale",  
      .onClick = getToggleWorldStateSetStr("tools", "manipulator-mode", "scale"), 
      .mappingId = radioButtonMappingId++ 
    },
    RadioButton { 
      .selected = manipulatorMode == "rotate", 
      .onClick = getToggleWorldStateSetStr("tools", "manipulator-mode", "rotate"),
      .mappingId = radioButtonMappingId++ 
    },
  };
  return radioButtonTest;
}

