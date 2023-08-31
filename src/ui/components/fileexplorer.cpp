#include "./fileexplorer.h"

enum FileContentType { File, Directory };
struct FileContent {
  FileContentType type;
  std::string content;
};
struct FileExplorer {
  int contentOffset;
  std::vector<std::string> currentPath;
  std::vector<FileContent> currentContents;
};

FileExplorer testExplorer {
  .contentOffset = 1,
  .currentPath = { "home", "desktop", "textures" },
  .currentContents  = {
    FileContent { .type = Directory, .content = ".." },
    FileContent { .type = File, .content = "wood.jpg" },\
    FileContent { .type = Directory, .content = "cool-textures" },
    FileContent { .type = File, .content = "bluetransparent.jpg" },
    FileContent { .type = File, .content = "notareal-file.jpg" },
  }
};

Component fileexplorerComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto listItemPtr = typeFromProps<std::vector<ListComponentData>>(props, listItemsSymbol);
    modassert(listItemPtr, "invalid listItems prop");
 
    auto listItems = *listItemPtr;


    std::vector<Component> elements;

    auto titleValue = strFromProp(props, titleSymbol, "title placeholder");
    auto titleTextbox = withPropsCopy(textbox, Props {
      .props = {
        PropPair { .symbol = valueSymbol, .value = titleValue },
      }
    });
    elements.push_back(titleTextbox);

    // current path
    { 
      std::vector<Component> pathElements;
      for (int i = 0 ; i < testExplorer.currentPath.size(); i++){
        auto value = testExplorer.currentPath.at(i);
        Props listItemProps {
          .props = {
            PropPair { .symbol = valueSymbol, .value = value },
            PropPair { .symbol = tintSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.f) },
            PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.8f) },
            PropPair { .symbol = paddingSymbol, .value = 0.01f },
          },
        };
        auto listItemWithProps = withPropsCopy(listItem, listItemProps);
        pathElements.push_back(listItemWithProps);
      }
      auto pathComponent = simpleHorizontalLayout(pathElements);
      elements.push_back(pathComponent);
    }
    /////////////////////
    // files
    {
      std::vector<Component> pathElements;
      for (int i = testExplorer.contentOffset ; ((i < (testExplorer.contentOffset + 10)) && i < testExplorer.currentContents.size()); i++){
        auto value = testExplorer.currentContents.at(i);
        Props listItemProps {
          .props = {
            PropPair { .symbol = valueSymbol, .value = (value.type == File ? value.content : std::string("[D] " + value.content)) },
            PropPair { .symbol = tintSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.f) },
            PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.8f) },
            PropPair { .symbol = paddingSymbol, .value = 0.01f },
          },
        };
        auto listItemWithProps = withPropsCopy(listItem, listItemProps);
        pathElements.push_back(listItemWithProps);
      }
      auto pathComponent = simpleVerticalLayout(pathElements, glm::vec2(0.5f, 0.5f), AlignmentParams {
        .layoutFlowHorizontal = UILayoutFlowNone2,
        .layoutFlowVertical = UILayoutFlowNegative2,

      });
      elements.push_back(pathComponent);
    }
    ///////////////


    std::vector<Component> choiceElements;
    for (int i = 0 ; i < listItems.size(); i++){
      ListComponentData& listItemData = listItems.at(i);
      auto onClick = listItemData.onClick.has_value() ? listItemData.onClick.value() : []() -> void {};
      Props listItemProps {
        .props = {
          PropPair { .symbol = valueSymbol, .value = listItemData.name },
          PropPair { .symbol = onclickSymbol, .value = onClick },
          PropPair { .symbol = tintSymbol, .value = glm::vec4(0.2f, 0.2f, 0.2f, 0.8f) },
        },
      };
      auto listItemWithProps = withPropsCopy(listItem, listItemProps);
      choiceElements.push_back(listItemWithProps);
    }

    auto horizontalComponent = simpleHorizontalLayout(choiceElements);
    elements.push_back(horizontalComponent);
    /////////////////////////


    auto listBoundingBox = simpleVerticalLayout(elements).draw(drawTools, props);
    //drawDebugBoundingBox(drawTools, listBoundingBox, glm::vec4(0.f, 1.f, 0.f, 1.f));
    auto boundingBox = listBoundingBox;
    auto sides = calculateSides(boundingBox);

    auto onClickX = fnFromProp(props, onclickSymbol);
    if (onClickX.has_value()){
      auto xMappingId = uniqueMenuItemMappingId();
      bool xHovered =  drawTools.selectedId.has_value() && drawTools.selectedId.value() == xMappingId;
      drawTextLeftHorzDownVert(drawTools, "x", sides.right, sides.top, 0.04f, xHovered ? glm::vec4(1.f, 1.f, 1.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 0.4f), xMappingId);
      drawTools.registerCallbackFns(xMappingId, onClickX.value());
    }

    //drawTools.drawText("some value", sides.right, sides.top, fontSize, false, std::nullopt, std::nullopt, true, std::nullopt, uniqueMenuItemMappingId());
    return boundingBox;
  },
};

