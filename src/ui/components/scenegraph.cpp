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

ScenegraphItem* getScenegraphItem(std::vector<ScenegraphItem>& scenegraphItems, std::vector<int> path){
	ScenegraphItem* item = NULL;
	std::vector<ScenegraphItem>* scenegraphItemsPtr = &scenegraphItems;
	for (auto pathIndex : path){
		item = &(scenegraphItemsPtr -> at(pathIndex));
		scenegraphItemsPtr = &(item -> children);
	}
	modassert(item, "getScenegraphItem item is null");
	return item;
}

Scenegraph createScenegraph(){
	auto depGraph = gameapi -> scenegraph();

	std::vector<ScenegraphItem> scenegraphItems = {};
	std::map<objid, objid> childToParent = {};
	std::map<objid, std::string> idToName = {};

	std::cout << "dep graph: " << std::endl;
	for (auto &dep : depGraph){
		childToParent[dep.childId] = dep.parentId;
		std::cout << "(" << dep.childId << ", " << dep.parentId << "),  ";
		idToName[dep.childId] = dep.child;
	}
	std::cout << std::endl;


	std::map<objid, std::vector<int>> idToScenegraphItemPath = {};
	const int LOOPMAX = 10000;
	int i = 0;
	while(i < LOOPMAX && childToParent.size() > 0){
		i++;
		std::optional<objid> idToErase = std::nullopt;
		for (auto &[childId, parentId] : childToParent){
			modassert(childId != parentId, "child is a parent of itself");
			if (parentId == 0){
				std::string label = idToName.at(childId);
				scenegraphItems.push_back(ScenegraphItem {
					.label = label,
					.expanded = true,
					.children = {},
				});
				int index = scenegraphItems.size() - 1;
				idToScenegraphItemPath[childId] = { index };
				idToErase = childId;
				break;
			}else if (idToScenegraphItemPath.find(parentId) != idToScenegraphItemPath.end()){
				modassert(idToScenegraphItemPath.find(parentId) != idToScenegraphItemPath.end(), "parent id not in list");
				auto path = idToScenegraphItemPath.at(parentId);
				ScenegraphItem* parentItem = getScenegraphItem(scenegraphItems, path);
				std::string label = idToName.at(childId);
				parentItem -> children.push_back(ScenegraphItem{
					.label = label,
					.expanded = true,
					.children = {},
				});
				path.push_back(parentItem -> children.size() - 1);

				idToScenegraphItemPath[childId] = path;
				idToErase = childId;
				break;
			}
		}
		if (idToErase.has_value()){
			childToParent.erase(idToErase.value());
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

void createScenegraphItem(Scenegraph& scenegraph, ScenegraphItem& item, int depth, std::vector<int> path, std::function<void(int)> onClick, std::function<bool(Component&)> addElement){
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
  	toggleScenegraphElement(scenegraph, path);
  };

  std::function<void(int)> onClickFn2 = [onClick, &item](int) -> void {
  	onClick(1000);
  };

  listItemProps.props.push_back(PropPair { .symbol = onclickSymbol, .value = onClickFn });
  listItemProps.props.push_back(PropPair { .symbol = onclickRightSymbol, .value = onClickFn2 });

  auto listItemElement = withPropsCopy(listItem, listItemProps);
  if (item.expanded && item.children.size() > 0){
  	bool limitReached = addElement(listItemElement);
  	if (limitReached){
  		return;
  	}
  	for (int i = 0; i < item.children.size(); i++){
  		ScenegraphItem& sceneItem = item.children.at(i);
  		auto elementPath = path;
  		elementPath.push_back(i);
  		createScenegraphItem(scenegraph, sceneItem, depth + 1, elementPath, onClick, addElement);
  	}
  }else{
  	addElement(listItemElement);
  }
}


Component scenegraphComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
  	Scenegraph** scenegraphPtr = typeFromProps<Scenegraph*>(props, valueSymbol);
  	modassert(scenegraphPtr, "scenegraph not provided");
  	auto scenegraph = *scenegraphPtr;

  	auto onClick = typeFromProps<std::function<void(int)>>(props, onclickSymbol);
  	modassert(onClick, "scenegraph onclick not provided");

    int offset = intFromProp(props, offsetSymbol, 0);
    if (offset < 0){
      offset = 0;
    }

    std::vector<Component> elements;

    int currentOffset = 0;
  	for (int i = 0; i < scenegraph -> items.size(); i++){
  		ScenegraphItem& item = scenegraph -> items.at(i);
    	createScenegraphItem(*scenegraph, item, 0, { i }, *onClick, [&elements, &currentOffset, &offset](Component& component) -> bool { 
    		currentOffset++;
    		if (currentOffset < offset){
    			return false;
    		}
    		if (elements.size() >= 10){
    			return true;
    		}
    		elements.push_back(component); 
    		return false;
    	});
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