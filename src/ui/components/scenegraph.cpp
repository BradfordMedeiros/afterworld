#include "./scenegraph.h"

Scenegraph testScenegraph {
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


extern CustomApiBindings* gameapi;
Scenegraph createScenegraph(){
	auto depGraph = gameapi -> scenegraph();

	std::vector<ScenegraphItem> scenegraphItems;
	std::map<objid, objid> childToParent;
	std::map<objid, std::string> idToName;
	for (auto &dep : depGraph){
		childToParent[dep.childId] = dep.parentId;
		idToName[dep.childId] = dep.child;
	}

	std::map<objid, ScenegraphItem*> idToScenegraphItem;
	const int LOOPMAX = 10000;
	int i = 0;
	while(i < LOOPMAX && childToParent.size() > 0){
		i++;
		for (auto &[childId, parentId] : childToParent){
			if (parentId == 0){
				scenegraphItems.push_back(ScenegraphItem {
					.label = idToName.at(childId),
					.expanded = true,
					.children = {},
				});
				idToScenegraphItem[childId] = &scenegraphItems.at(scenegraphItems.size() -1);
				childToParent.erase(childId);
				break;
			}else if (idToScenegraphItem.find(parentId) != idToScenegraphItem.end()){
				ScenegraphItem* parentItem = idToScenegraphItem.at(parentId);
				parentItem -> children.push_back(ScenegraphItem{
					.label = idToName.at(childId),
					.expanded = true,
					.children = {},
				});
				ScenegraphItem* childItem = &(parentItem -> children.at(parentItem -> children.size() - 1));
				idToScenegraphItem[childId] = childItem;
				childToParent.erase(childId);
				break;
			}
		}
	}
	modassert(childToParent.size() == 0, "create scenegraph - invalid graph, some orphaned elements");
	return Scenegraph  {
		.items = scenegraphItems,
	};
	return testScenegraph;
}

void toggleScenegraphElement(Scenegraph& scenegraph, std::vector<int> path){
	if (path.size() == 0){
		return;
	}
	ScenegraphItem* item = &scenegraph.items.at(path.at(0));
	for (int i = 1; i < path.size(); i++){
		item = &(item -> children.at(path.at(i)));
	}
	item -> expanded = !(item -> expanded);
}

Component createScenegraphItem(Scenegraph& scenegraph, ScenegraphItem& item, int depth, std::vector<int> path, std::function<void(int)> onClick){
  std::string prefix = "";
  for (int i = 0; i < depth; i++){
  	prefix += "    ";
  }
  std::string name = prefix + (item.expanded ? "-" : "x") + item.label;

  Props listItemProps {
    .props = {
      PropPair { .symbol = valueSymbol,   .value = name },
      PropPair { .symbol = paddingSymbol, .value = 0.01f },
      PropPair { .symbol = fontsizeSymbol, .value = 0.015f },
      //PropPair { .symbol = tintSymbol, .value = glm::vec4(0.1f, 0.1f, 0.1f, 1.f) },
      //PropPair { .symbol = colorSymbol, .value = color },
    },
  };

  bool callOnClick = item.children.size() == 0;
  std::function<void()> onClickFn = [&scenegraph, path, callOnClick, onClick]() -> void {
  	//std::cout << "dock path is: ";
  	for (auto index : path){
  		std::cout << index << " ";
  	}
  	std::cout << std::endl;
  	toggleScenegraphElement(scenegraph, path);
  };

  std::function<void(int)> onClickFn2 = [onClick, &item](int) -> void {
  	onClick(1000);
  };

  listItemProps.props.push_back(PropPair { .symbol = onclickSymbol, .value = onClickFn });
  listItemProps.props.push_back(PropPair { .symbol = onclickRightSymbol, .value = onClickFn2 });

  auto listItemElement = withPropsCopy(listItem, listItemProps);
  if (item.expanded && item.children.size() > 0){
  	std::vector<Component> elements;
  	elements.push_back(listItemElement);
  	for (int i = 0; i < item.children.size(); i++){
  		ScenegraphItem& sceneItem = item.children.at(i);

  		auto elementPath = path;
  		elementPath.push_back(i);
  		//std::cout << "path is: " << print(elementPath) << std::endl;
  		auto component = createScenegraphItem(scenegraph, sceneItem, depth + 1, elementPath, onClick);
  		elements.push_back(component);
  	}

		auto layout = simpleVerticalLayout(elements);
		return layout;
  }

  return listItemElement;
}


Component scenegraphComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
  	Scenegraph** scenegraphPtr = typeFromProps<Scenegraph*>(props, valueSymbol);
  	modassert(scenegraphPtr, "scenegraph not provided");
  	auto scenegraph = *scenegraphPtr;

  	auto onClick = typeFromProps<std::function<void(int)>>(props, onclickSymbol);
  	modassert(onClick, "scenegraph onclick not provided");

    std::vector<Component> elements;
  	for (int i = 0; i < scenegraph -> items.size(); i++){
  		ScenegraphItem& item = scenegraph -> items.at(i);
    	elements.push_back(createScenegraphItem(*scenegraph, item, 0, { i }, *onClick));
  	}
  	Layout layout {
  	  .tint = glm::vec4(0.f, 0.f, 0.f, 1.f),
  	  .showBackpanel = true,
  	  .borderColor = std::nullopt,
  	  .minwidth = 0.5f,
  	  .minheight = 0.5f,
  	  .layoutType = LAYOUT_VERTICAL2,
  	  .layoutFlowHorizontal = UILayoutFlowNone2,
  	  .layoutFlowVertical = UILayoutFlowNegative2,
  	  .alignHorizontal = UILayoutFlowNegative2,
  	  .alignVertical = UILayoutFlowPositive2,
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
  	auto layoutScenegraph = withPropsCopy(layoutComponent, listLayoutProps);
  	return layoutScenegraph.draw(drawTools, props);
  },
};