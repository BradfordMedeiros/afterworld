#include "./editorview.h"

glm::vec4 colorPickerColor(0.f, 0.f, 0.f, 1.f);

TextData newSceneTextData {
  .valueText = "",
  .cursorLocation = 0,
  .highlightLength = 0,
  .maxchars = -1,
};

Component editorViewComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    EditorViewOptions* editorOptions = typeFromProps<EditorViewOptions>(props, valueSymbol);

    for (auto &dockedDock : (*editorOptions -> dockedDocks)){
      Props dockProps { 
        .props = {
          { dockTypeSymbol, dockedDock }, 
          { xoffsetFromSymbol, 1.5f },
        }
      };    
      auto dock = withProps(dockComponent, dockProps);
      auto defaultWindowProps = getDefaultProps();

      auto windowDockSymbol = getSymbol(std::string("window-symbol-") + dockedDock);

      std::set<std::string>* dockedDocksPtr = editorOptions -> dockedDocks;
      auto dockDockPtr = &dockedDock;
      std::function<void()> onClickX = [editorOptions, dockedDocksPtr, dockDockPtr]() -> void {
        dockedDocksPtr -> erase(*dockDockPtr);
      };

      createUiWindow(
        dock, 
        windowDockSymbol, 
        onClickX, 
        dockedDock, 
        AlignmentParams { 
          .layoutFlowHorizontal = UILayoutFlowNegative2, .layoutFlowVertical = UILayoutFlowNegative2 
        }
      ).draw(drawTools, defaultWindowProps);
    }

    Props sceneManagerProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = editorOptions -> sceneManagerInterface },
        PropPair { .symbol = xoffsetSymbol, .value = -0.83f },
        PropPair { .symbol = yoffsetSymbol, .value = 0.9f },
      },
    };
    scenemanagerComponent.draw(drawTools, sceneManagerProps);

    // Pop up windows.... this probably could be org better
    // color picker /////////////////////
    auto onNewColor = editorOptions -> onNewColor;

		std::function<void(glm::vec4)> onSlide = [onNewColor](glm::vec4 value) -> void {
		  colorPickerColor = value;
		  if (onNewColor.has_value()){
		    onNewColor.value()(colorPickerColor);
		  }
		};
    auto colorPicker = withPropsCopy(colorPickerComponent, Props {
      .props = { 
        PropPair { onSlideSymbol,  onSlide },
        PropPair { tintSymbol, colorPickerColor },
      }
    });
    auto uiWindowComponent = createUiWindow(
      colorPicker, 
      windowColorPickerSymbol, 
      []() -> void { windowSetEnabled(windowColorPickerSymbol, false); }, 
      *(editorOptions -> colorPickerTitle)
    );
    auto defaultWindowProps = getDefaultProps();
    uiWindowComponent.draw(drawTools, defaultWindowProps);
    ///////////////////////////////////////////////

    auto onFileAddedFn = editorOptions -> onFileAddedFn;
    { // file select 
      FileCallback onFileSelect = [onFileAddedFn](bool isDirectory, std::string file) -> void {
        modassert(!isDirectory, "on file select gave something not a file");
        modlog("dock - file select", std::string(isDirectory ? "dir" : "file") + " " + file);
        if (onFileAddedFn.has_value()){
          onFileAddedFn.value()(false, file);
        }
      };
      Props filexplorerProps {
        .props = {
          PropPair { .symbol = fileExplorerSymbol, .value = testExplorer },
          PropPair { .symbol = fileChangeSymbol, .value = onFileSelect },
          PropPair { .symbol = offsetSymbol, .value = editorOptions -> fileexplorerScrollAmount },
        },
      };
      if (editorOptions -> fileFilter.has_value()){
        filexplorerProps.props.push_back(
          PropPair { .symbol = fileFilterSymbol, .value = editorOptions -> fileFilter.value() }
        );
      }
      auto fileExplorer = withProps(fileexplorerComponent, filexplorerProps);
      auto fileExplorerWindow = createUiWindow(fileExplorer, windowFileExplorerSymbol, []() -> void { windowSetEnabled(windowFileExplorerSymbol, false); }, "File Explorer");
      auto defaultWindowProps = getDefaultProps();
      fileExplorerWindow.draw(drawTools, defaultWindowProps);
    }
    ////////////////////////////////////////////////////
    {
      auto onInputBoxFn = editorOptions -> onInputBoxFn;
      std::vector<ListComponentData> dialogOptions = {
        ListComponentData {
          .name = "confirm",
          .onClick = [onInputBoxFn]() -> void {
            std::cout << "dialog confirm on click" << std::endl;
            if (onInputBoxFn.has_value()){
              onInputBoxFn.value()(false, newSceneTextData.valueText);
            }
          },      
        },
        ListComponentData {
          .name = "cancel",
          .onClick = [onInputBoxFn]() -> void {
            if (onInputBoxFn.has_value()){
              onInputBoxFn.value()(true, "");
            }
          },      
        },
      };
      {
        std::function<void(TextData, int)> onEdit = [](TextData textData, int rawKey) -> void {
          newSceneTextData = textData;
        };
        Props dialogProps {
          .props = {
            PropPair { .symbol = listItemsSymbol, .value = dialogOptions },
            PropPair { .symbol = detailSymbol, .value = std::string("Enter Name of New Scene") },
            PropPair { .symbol = valueSymbol, .value =  newSceneTextData },
            PropPair { .symbol = onInputSymbol, .value = onEdit },
          },
        };
        auto dialogWithProps = withPropsCopy(dialogComponent, dialogProps);
        auto dialogWindow = createUiWindow(dialogWithProps, windowDialogSymbol, []() -> void { windowSetEnabled(windowDialogSymbol, false); }, "New Scene");
        auto defaultProps = getDefaultProps();
        dialogWindow.draw(drawTools, defaultProps);
      }
    }

    /////////////////////
    {
      auto imageListDatas = editorOptions -> imageListDatas;
      std::function<void(int)> onImageClick = [onFileAddedFn, imageListDatas](int index) -> void {
        if (onFileAddedFn.has_value()){
          onFileAddedFn.value()(false, imageListDatas -> images.at(index).image);
        }
      };
      auto imageListComponent = withPropsCopy(imageList, Props {
        .props = { 
          PropPair { imagesSymbol,  *imageListDatas },
          PropPair { onclickSymbol, onImageClick },
          PropPair { offsetSymbol,  editorOptions -> imageListScrollAmount },
        }
      });
      auto uiWindowComponent = createUiWindow(imageListComponent, windowImageExplorerSymbol, []() -> void { windowSetEnabled(windowImageExplorerSymbol, false); }, "Image Explorer");
      auto defaultWindowProps = getDefaultProps();
      uiWindowComponent.draw(drawTools, defaultWindowProps);
    }

    /////////////////

    auto defaultProps = getDefaultProps();
    withProps(navList, navListProps).draw(drawTools, defaultProps);

    {
      Props navbarProps {
        .props = {
          { onclickSymbol, editorOptions -> onClickNavbar },
          { valueSymbol, editorOptions -> navbarType },
        }
      };
      navbarComponent.draw(drawTools, navbarProps);
    }

    Props worldPlayProps {
       .props = {
         PropPair { .symbol = xoffsetSymbol, .value = 0.f },
         PropPair { .symbol = yoffsetSymbol, .value = -1.f },
         PropPair { .symbol = valueSymbol, .value = editorOptions -> worldPlayInterface },
       }
    };
    worldplay.draw(drawTools, worldPlayProps);

    if (editorOptions -> debugConfig.has_value()){
      Props props {
        .props = {
          PropPair { .symbol = valueSymbol, .value = editorOptions -> debugConfig.value() },
          PropPair { .symbol = xoffsetSymbol, .value = -1.f },
        },
      };
      debugComponent.draw(drawTools, props);
    }

    return { .x = 0, .y = 0, .width = 0.f, .height = 0.f };
  },
};

