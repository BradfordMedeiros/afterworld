#include "./navlist.h"

extern CustomApiBindings* gameapi;
extern NavListApi navListApi;
extern UiManagerContext uiManagerContext;
void setMenuBackground(std::string background);


int mappingId = 999995;
std::vector<NestedListItem> nestedListTest = {
  NestedListItem {
    .item = ImListItem {
      .value = "file",
      .onClick = []() -> void { },
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
          .onClick = std::nullopt, 
          .mappingId = mappingId++,
        },
        .items = {
          NestedListItem {
            .item = ImListItem {
              .value = "main",
              .onClick = []() -> void {
                navListApi.changeLayout("main");
              },
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "gun editor",
              .onClick = []() -> void {
                navListApi.changeLayout("gameplay");
              }, 
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "editor",
              .onClick = []() -> void {
                navListApi.changeLayout("editor");
              }, 
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
      .value = "render",
      .onClick = []() -> void { },
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
      .onClick = []() -> void { },
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
      .onClick = []() -> void { },
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
      .onClick = []() -> void { }, 
      .mappingId = mappingId++,
    },
    .items = {
      NestedListItem {
        .item = ImListItem {
          .value = "new scene",
          .onClick = []() -> void {
            uiManagerContext.uiMainContext.openNewSceneMenu([](bool closedWithoutInput, std::string sceneName) -> void {
              if (!closedWithoutInput){
                uiManagerContext.uiContext -> newScene(sceneName);
              }
            });
          },
          .mappingId = mappingId++,
        },
        .items = {},
      },
      NestedListItem {
        .item = ImListItem {
          .value = "save scene",
          .onClick = []() -> void {
            modassert(uiManagerContext.uiContext, "uiContextPtr null");
            uiManagerContext.uiContext -> worldPlayInterface.saveScene();
          },
          .mappingId = mappingId++,
        },
        .items = {},
      },
      NestedListItem {
        .item = ImListItem {
          .value = "reset scene",
          .onClick = []() -> void {
            modassert(uiManagerContext.uiContext, "uiContextPtr null");
            uiManagerContext.uiContext -> resetScene();
          },
          .mappingId = mappingId++,
        },
        .items = {},
      },
    },
  },
  NestedListItem {
    .item = ImListItem {
      .value = "misc fun",
      .onClick = []() -> void { }, 
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
                setMenuBackground("../gameresources/textures/backgrounds/test3.png");
              },
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "dark-trippy-invert",
              .onClick = []() -> void {
                setMenuBackground("../gameresources/textures/backgrounds/test5.png");
              },
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "tvs",
              .onClick = []() -> void {
                setMenuBackground("../gameresources/textures/backgrounds/tvs.png");
              },
              .mappingId = mappingId++,
            },
            .items = {},
          },
          NestedListItem {
            .item = ImListItem {
              .value = "arcade",
              .onClick = []() -> void {
                setMenuBackground("../gameresources/textures/backgrounds/arcade2.png");
              },
              .mappingId = mappingId++,
            },
            .items = {},
          },
        },
      },
      NestedListItem {
        .item = ImListItem {
          .value = "discord",
          .onClick = []() -> void {
            system("xdg-open https://discord.com/channels/1135105623688757288/");
          },
          .mappingId = mappingId++,
        },
        .items = {},
      },
    },
  },
};

Props nestedListProps { 
  .props = {
    PropPair { .symbol = getSymbol("items"), .value = nestedListTest }
    
  }
};
Component navList = withProps(nestedList, nestedListProps);

Props navListProps { 
  .props = {
    PropPair {
      .symbol = tintSymbol,
      .value = glm::vec4(0.f, 0.f, 0.f, 0.8f),
    },
    PropPair {
      .symbol = minwidthSymbol,
      .value = 0.15f,
    },
    PropPair {
      .symbol = xoffsetSymbol,
      .value = -0.99f,
    },
    PropPair {
      .symbol = yoffsetSymbol,
      .value = 0.f,  
    }
  }
};

