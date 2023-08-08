#include "./imagelist.h"

Component createImageList(ImageList& imageList){
	Component imageListTest  {
	  .draw = [&imageList](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
	  	drawTools.drawRect(0.f + 0.2f, 0.f, 1.f, 1.f, false, glm::vec4(0.f, 0.f, 1.f, 0.4f), std::nullopt, true, std::nullopt, std::nullopt);
	  	for (int i = 0; i < imageList.images.size(); i++){
	  		auto selectedId = getGlobalState().selectedId;
	  		bool selected = selectedId.has_value() && (selectedId.value() == imageList.mappingId + i);
	    	int column = i % 4;
	    	int row = i / 4;
	    	drawTools.drawRect(column * 0.2f , row * 0.2f + row * 0.02f, 0.2f, 0.2f, false, selected ? glm::vec4(2.f, 2.f, 2.f, 1.f) : std::optional<glm::vec4>(std::nullopt), std::nullopt, true, imageList.mappingId + i, imageList.images.at(i));
	  	}

	  	BoundingBox2D boundingBox {
	  	  .x = 0,
	  	  .y = 0,
	  	  .width = 1.f,
	  	  .height = 1.f,
	  	};
      drawDebugBoundingBox(drawTools, boundingBox);
	  	return boundingBox;
	  },
	  .imMouseSelect = [](std::optional<objid> mappingIdSelected, Props& props) -> void {
	     
	  }  
	};
	return imageListTest;
}
