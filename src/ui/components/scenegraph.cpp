#include "./scenegraph.h"

struct ScenegraphItem {
	std::string label;
	bool expanded;
	std::vector<ScenegraphItem> children;
};

struct Scenegraph {
	std::vector<ScenegraphItem> items;
};

Scenegraph scenegraph {
	.items = {
		ScenegraphItem {
			.label = "item1",
			.expanded = true,
			.children = {
				ScenegraphItem {
					.label = "item1-child1",
					.expanded = true,
					.children = {
						ScenegraphItem {
							.label = "item11-child1",
							.expanded = true, 
							.children = {}
						},
					}
				},
				ScenegraphItem {
					.label = "item1-child2", 
					.children = {}
				},
			}
		},
		ScenegraphItem {
			.label = "item2", 
			.expanded = false,
			.children = {
				ScenegraphItem {
					.label = "item2-child1", 
					.children = {}
				},
			}
		},
	}
};

Component createScenegraphItem(ScenegraphItem& item, int depth, std::vector<int> path){
 	bool expanded = true;

  std::string prefix = "";
  for (int i = 0; i < depth; i++){
  	prefix += "    ";
  }
  std::string name = prefix + item.label;

  Props listItemProps {
    .props = {
      PropPair { .symbol = valueSymbol,   .value = expanded ? (std::string("- ") + name) : (std::string("+ ") + name) },
      PropPair { .symbol = paddingSymbol, .value = 0.01f },
      PropPair { .symbol = fontsizeSymbol, .value = 0.01f },


      //PropPair { .symbol = tintSymbol, .value = tint },
      //PropPair { .symbol = colorSymbol, .value = color },
    },
  };
  //if (onClick.has_value()){
  //  listItemProps.props.push_back(PropPair { .symbol = onclickSymbol, .value = onClick.value() });
  //}
  auto listItemElement = withPropsCopy(listItem, listItemProps);
  if (item.expanded && item.children.size() > 0){
  	std::vector<Component> elements;
  	elements.push_back(listItemElement);
  	for (int i = 0; i < item.children.size(); i++){
  		ScenegraphItem& sceneItem = item.children.at(i);

  		auto elementPath = path;
  		elementPath.push_back(i);
  		std::cout << "path is: " << print(elementPath) << std::endl;
  		auto component = createScenegraphItem(sceneItem, depth + 1, elementPath);
  		elements.push_back(component);
  	}
		auto layout = simpleVerticalLayout(elements);
		return layout;
  }

  return listItemElement;
}


Component scenegraphComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    std::vector<Component> elements;
  	for (auto &item : scenegraph.items){
    	elements.push_back(createScenegraphItem(item, 0, {}));
  	}
		auto layout = simpleVerticalLayout(elements);
  	return layout.draw(drawTools, props);
  },
};