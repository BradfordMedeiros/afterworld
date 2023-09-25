#include "./scenemanager.h"


const int maxScenes = 5;
Component scenemanagerComponent  {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    SceneManagerInterface* sceneManagerInterface = typeFromProps<SceneManagerInterface>(props, valueSymbol);

    Props listItemProps {
    	.props = {
    		PropPair { .symbol = valueSymbol, .value = sceneManagerInterface -> currentScene >= 0 ? sceneManagerInterface -> scenes.at(sceneManagerInterface -> currentScene) : "NONE" },
    		PropPair { .symbol = paddingSymbol, .value = 0.02f },
    		PropPair { .symbol = onclickSymbol, .value = sceneManagerInterface -> toggleShowScenes },
    	}
    };

    auto currentSceneComponent = withProps(listItem, listItemProps);
  	

  	Props leftSceneArrowProps {
    	.props = {
    		PropPair { .symbol = valueSymbol, .value = std::string("Scene") },
    		PropPair { .symbol = tintSymbol, .value = glm::vec4(0.2f, 0.2f, 0.2f, .5f) },
    		PropPair { .symbol = paddingSymbol, .value = 0.02f },
        PropPair { .symbol = limitSymbol, .value = 40 },
    	}
    };

    auto leftSceneArrowComponent = withProps(listItem, leftSceneArrowProps);

    std::vector<Component> components;
    components.push_back(leftSceneArrowComponent);
    components.push_back(currentSceneComponent);

    auto selectorComponent =  simpleHorizontalLayout(components);

    std::vector<Component> verticalElements = { selectorComponent };

    if (sceneManagerInterface -> showScenes){
	    std::vector<Component> sceneElements;

      int totalElements = 0;
    	for (int  i = sceneManagerInterface -> offset; i < sceneManagerInterface -> scenes.size() && totalElements < maxScenes; i++){
        totalElements++;

        auto onSelectScene = sceneManagerInterface -> onSelectScene;

        auto scene = sceneManagerInterface -> scenes.at(i);
        std::function<void()> onClick = [onSelectScene, scene, i]() -> void {
          onSelectScene(i, scene);
        };
    		Props listItemProps {
    			.props = {
    				PropPair { .symbol = valueSymbol, .value = scene },
    				PropPair { .symbol = paddingSymbol, .value = 0.02f },
            PropPair { .symbol = onclickSymbol, .value = onClick },
            PropPair { .symbol = limitSymbol, .value = 40 },
    			}
    		};
    		if (i == sceneManagerInterface -> currentScene){
    			listItemProps.props.push_back(PropPair { .symbol = tintSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 1.f) });
    			listItemProps.props.push_back(PropPair { .symbol = colorSymbol, .value = glm::vec4(0.f, 0.f, 0.f, 1.f) });
    		}
    		auto sceneNameComponent = withPropsCopy(listItem, listItemProps);
    		sceneElements.push_back(sceneNameComponent);
    	}
	    auto sceneList = simpleVerticalLayout(sceneElements);
	    verticalElements.push_back(sceneList);
    }

    auto selectorWithList = simpleVerticalLayout(
    	verticalElements, 
    	glm::vec2(0.f, 0.f), 
    	AlignmentParams {
  			.layoutFlowHorizontal = UILayoutFlowNegative2,
  			.layoutFlowVertical = UILayoutFlowNegative2,
    	}
    );

    return selectorWithList.draw(drawTools, props);
  },
};

