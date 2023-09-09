#include "./imagelist.h"

const int imagesSymbol = getSymbol("images");

const int numPerRow = 5;
Component imageList  {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto measurer = createMeasurer();
  	auto imageList = typeFromProps<ImageList>(props, imagesSymbol);
    auto onClickPtr = typeFromProps<std::function<void(int)>>(props, onclickSymbol);
    modassert(onClickPtr, "invalid prop onclick imagelist");
    auto onClick = *onClickPtr;
  	modassert(imageList, "invalid image list prop");
  	for (int i = 0; i < imageList -> images.size(); i++){
      auto mappingId = uniqueMenuItemMappingId();
  		bool selected = drawTools.selectedId.has_value() && drawTools.selectedId.has_value() && (drawTools.selectedId.value() == mappingId);
    	int column = i % numPerRow;
    	int row = i / numPerRow;

      float width = 0.1f;
      float height = 0.1f;
      float x = column * width;
      float y = row * height + row * 0.02f;

    	drawTools.drawRect(x, y, width, height, false, selected ? glm::vec4(2.f, 2.f, 2.f, 1.f) : std::optional<glm::vec4>(std::nullopt), std::nullopt, true, mappingId, imageList -> images.at(i));

      drawTools.registerCallbackFns(mappingId, [onClick, i]() -> void {
        onClick(i);
      });

      setBox(measurer, x, y, width, height);
  	}
    auto boundingBox = measurerToBox(measurer);
    //drawDebugBoundingBox(drawTools, boundingBox);
  	return boundingBox;
  },
};

