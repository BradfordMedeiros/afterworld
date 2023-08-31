#include "./fileexplorer.h"

const int fileExplorerSymbol = getSymbol("file-explorer");

typedef std::function<void(std::string)> ExplorerNavigation;

Component fileexplorerComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto fileExplorer = typeFromProps<FileExplorer>(props, fileExplorerSymbol);
    modassert(fileExplorer, "fileexplorer must be defined for dialog");

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

    auto onChange = fileExplorer -> explorerOnChange;

    // current path
    { 
      std::vector<Component> pathElements;
      for (int i = 0 ; i < fileExplorer -> currentPath.size(); i++){
        auto value = fileExplorer -> currentPath.at(i);

        std::function<void()> onClick = [onChange]() -> void {
          std::cout << "on click" << std::endl;
          onChange(Directory);
        };
        Props listItemProps {
          .props = {
            PropPair { .symbol = valueSymbol, .value = value },
            PropPair { .symbol = tintSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.f) },
            PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.8f) },
            PropPair { .symbol = paddingSymbol, .value = 0.01f },
            PropPair { .symbol = onclickSymbol, .value = onClick },
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
      for (int i = fileExplorer -> contentOffset ; ((i < (fileExplorer -> contentOffset + 10)) && i < fileExplorer -> currentContents.size()); i++){
        auto value = fileExplorer -> currentContents.at(i);
        auto type = value.type;
        std::function<void()> onClick = [onChange, type]() -> void {
          std::cout << "on click" << std::endl;
          onChange(type);
        };
        Props listItemProps {
          .props = {
            PropPair { .symbol = valueSymbol, .value = (value.type == File ? value.content : std::string("[D] " + value.content)) },
            PropPair { .symbol = tintSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.f) },
            PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.8f) },
            PropPair { .symbol = paddingSymbol, .value = 0.01f },
            PropPair { .symbol = onclickSymbol, .value = onClick },
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

    {
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
    }
    /////////////////////////

    auto boundingBox = simpleVerticalLayout(elements).draw(drawTools, props);
    auto onClickX = fnFromProp(props, onclickSymbol);
    if (onClickX.has_value()){
      drawWindowX(drawTools, boundingBox, onClickX.value());
    }
    return boundingBox;
  },
};

