#include "./worldplay.h"

Component worldplay {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
  	WorldPlayInterface** worldPlayInterfacePtr = typeFromProps<WorldPlayInterface*>(props, valueSymbol);
  	modassert(worldPlayInterfacePtr, "worldplay : interface not provided");

		WorldPlayInterface* worldPlayInterface = *worldPlayInterfacePtr;
  	ImageList imageListData {
 			.images = {
 				ImageListImage {
 					.image = !worldPlayInterface -> isGameMode() ?  "./res/scenes/editor/dock/images/play.png" : "./res/scenes/editor/dock/images/pause.png",
 				},
 				ImageListImage {
 					.image = worldPlayInterface -> isPaused() ?  "./res/scenes/editor/dock/images/play.png" : "./res/scenes/editor/dock/images/pause.png",
 					.tint = worldPlayInterface -> isGameMode() ? std::optional<glm::vec4>(std::nullopt) : glm::vec4(0.5f, 0.5f, 0.5f, 1.f),
 				}
	  		 
 			}
		};


		std::function<void(int)> onClick = [worldPlayInterface](int index) -> void {
			if (index == 0){ // play button
				if (worldPlayInterface -> isGameMode()){
					worldPlayInterface -> exitGameMode();
				}else{
					worldPlayInterface -> enterGameMode();
				}
			}else if (index == 1){
				if (worldPlayInterface -> isGameMode()){
					 if (worldPlayInterface -> isPaused()){
					 		worldPlayInterface -> resume();
					 }else{
					 		worldPlayInterface -> pause();
					 }
				}
			}
		};
		Props imageProps {
			.props = {
        PropPair { imagesSymbol,  imageListData },
        PropPair { onclickSymbol, onClick },
        PropPair { fixedSizeSymbol, false },
        PropPair { sizeSymbol, 0.05f },
			},
		};

  	auto worldPlayInner = withProps(imageList, imageProps);
  	AlignmentParams align {
  		.layoutFlowHorizontal = UILayoutFlowNone2,
  		.layoutFlowVertical = UILayoutFlowPositive2,
  	};
  	return simpleLayout(worldPlayInner, glm::vec2(0.f, 0.f), align).draw(drawTools, props);
  },
};

