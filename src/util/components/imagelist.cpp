#include "./imagelist.h"

extern CustomApiBindings* gameapi;

ImageList imageList {
	.images = {
		"./res/textures/wood.jpg",
		"./res/textures/grass.jpg",
		"./res/textures/brickwall.jpg",
		"./res/textures/grid.png",
		"./res/textures/grass.jpg",
		"./res/textures/grid.png",
		"./res/textures/wood.jpg",
		"./res/textures/brickwall.jpg",
	},
};

Component imageListTest  {
  .draw = [](Props& props) -> BoundingBox2D {
  	gameapi -> drawRect(0.f + 0.2f, 0.f, 1.f, 1.f, false, glm::vec4(0.f, 0.f, 1.f, 0.4f), std::nullopt, true, std::nullopt, std::nullopt);
  	for (int i = 0; i < imageList.images.size(); i++){
    	int column = i % 4;
    	int row = i / 4;
    	gameapi -> drawRect(column * 0.2f , row * 0.2f + row * 0.02f, 0.2f, 0.2f, false, std::nullopt, std::nullopt, true, std::nullopt, imageList.images.at(i));
  	}
  	return BoundingBox2D {
  	  .x = 0,
  	  .y = 0,
  	  .width = 1.f,
  	  .height = 1.f,
  	};
  },
  .imMouseSelect = [](std::optional<objid> mappingIdSelected) -> void {
     
  }  
};