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
          .value = "- lighting",
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
        .items = {},
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
          .onClick = []() -> void {  }, // maybe this should send a request to core engine so can shutdown properly }
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
          .onClick = std::nullopt,
          .mappingId = mappingId++,
        },
        .items = {},
      },
      NestedListItem {
        .item = ImListItem {
          .value = "save scene",
          .onClick = std::nullopt,
          .mappingId = mappingId++,
        },
        .items = {},
      },
      NestedListItem {
        .item = ImListItem {
          .value = "reset scene",
          .onClick = std::nullopt, // maybe this should send a request to core engine so can shutdown properly }
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
              .value = "trippy",
              .onClick = std::nullopt,
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "fish",
              .onClick = std::nullopt,
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