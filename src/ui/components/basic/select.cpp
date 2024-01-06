#include "./select.h"

Component selectComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {  
   	auto selectOptions = typeFromProps<SelectOptions>(props, valueSymbol);
  	modassert(selectOptions, "select options not provided to select");

    int currentSelection = selectOptions -> currentSelection();
  	int selectedIndex = currentSelection >= 0 ? currentSelection : 0;

  	bool isExpanded = selectOptions -> isExpanded();
    std::vector<std::string>& options = selectOptions -> getOptions();

    std::function<void(bool)> toggleExpanded = selectOptions -> toggleExpanded;;

  	std::function<void()> onClick = [isExpanded, toggleExpanded]() -> void {
	  	toggleExpanded(!isExpanded);
	  };
    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = options.at(selectedIndex) + " [ ]"},
        PropPair { .symbol = onclickSymbol, .value = onClick },
        //PropPair { .symbol = colorSymbol, .value = selectedIndex == i ? glm::vec4(0.f, 0.f, 1.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f) },
        PropPair { .symbol = paddingSymbol, .value = styles.dockElementPadding },
      },
    };

    auto listItemWithProps = withPropsCopy(listItem, listItemProps);

    std::vector<Component> elements;
  	elements.push_back(listItemWithProps);

  	if (selectOptions -> isExpanded()){
  		for (int i = 0; i < options.size(); i++){

  			std::string& element = options.at(i);
 			  std::function<void(int, std::string&)> onSelect = selectOptions -> onSelect;

    		std::function<void()> onClick = [i, element, onSelect]() -> void {
    			std::string value = element;
    			onSelect(i, value);
	  		};
    	 	Props listItemProps {
    	 	 .props = {
    	 	   PropPair { .symbol = valueSymbol, .value = element },
    	 	   PropPair { .symbol = onclickSymbol, .value = onClick },
    	 	   PropPair { .symbol = colorSymbol, .value = styles.highlightColor },
		       PropPair { .symbol = paddingSymbol, .value = styles.dockElementPadding },
    	 	 },
    	 	};
    	 	auto listItemWithProps = withPropsCopy(listItem, listItemProps);
    	 	elements.push_back(listItemWithProps);
  		}  		
  	}



  	Layout layout {
  	  .tint = styles.primaryColor,
  	  .showBackpanel = true,
  	  .borderColor = glm::vec4(1.f, 1.f, 1.f, 0.f),
  	  .minwidth = 0.f,
  	  .minheight = 0.f,
  	  .layoutType =  LAYOUT_VERTICAL2,
  	  .layoutFlowHorizontal = UILayoutFlowNone2,
  	  .layoutFlowVertical = UILayoutFlowNone2,
  	  .alignHorizontal = UILayoutFlowNegative2,
  	  .alignVertical = UILayoutFlowNone2,
  	  .spacing = 0.f,
  	  .minspacing = 0.f,
  	  .padding = 0.f,
  	  .children = elements,
  	};
    Props listLayoutProps {
      .props = {
        { .symbol = layoutSymbol, .value = layout },
      },
    };
    auto listBoundingBox = withProps(layoutComponent, listLayoutProps).draw(drawTools, props);


    return listBoundingBox;
  },
};