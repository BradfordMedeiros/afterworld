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
          .value = "screen grid",
          .onClick = []() -> void { 
            getGlobalState().systemConfig.showScreenspaceGrid = !getGlobalState().systemConfig.showScreenspaceGrid; 
          },
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
        },
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

