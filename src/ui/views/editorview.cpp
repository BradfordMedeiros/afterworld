#include "./editorview.h"

glm::vec4 colorPickerColor(0.f, 0.f, 0.f, 1.f);

Component editorViewComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    EditorViewOptions* editorOptions = typeFromProps<EditorViewOptions>(props, valueSymbol);

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

    return { .x = 0, .y = 0, .width = 0.f, .height = 0.f };
  },
};

