#include "./imagelist.h"

const int imagesSymbol = getSymbol("images");

const int numPerRow = 5;
const int maxRows = 5;
Component imageList  {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto measurer = createMeasurer();
  	auto imageList = typeFromProps<ImageList>(props, imagesSymbol);
    auto onClickPtr = typeFromProps<std::function<void(int)>>(props, onclickSymbol);
    modassert(onClickPtr, "invalid prop onclick imagelist");
    auto onClick = *onClickPtr;
    int offset = intFromProp(props, offsetSymbol, 0);
    if (offset < 0){
      offset = 0;
    }

  	modassert(imageList, "invalid image list prop");
  	for (int i = offset; i < imageList -> images.size(); i++){
      auto mappingId = uniqueMenuItemMappingId();
  		bool selected = drawTools.selectedId.has_value() && drawTools.selectedId.has_value() && (drawTools.selectedId.value() == mappingId);
    	int column = (i - offset) % numPerRow;
    	int row = (i - offset) / numPerRow;
      if (row >= maxRows){
        break;
      }

      float width = 0.1f;
      float height = 0.1f;
      float x = column * width;
      float y = -1.f * row * height;

    	drawTools.drawRect(x, y, width, height, false, selected ? glm::vec4(2.f, 2.f, 2.f, 1.f) : std::optional<glm::vec4>(std::nullopt), std::nullopt, true, mappingId, imageList -> images.at(i));
      BoundingBox2D box {
        .x = x,
        .y = y,
        .width  = width,
        .height = height,
      };
      if (selected){
        drawDebugBoundingBox(drawTools, box, glm::vec4(1.f, 1.f, 1.f, 1.f));
      }
      drawTools.registerCallbackFns(mappingId, [onClick, i]() -> void {
        onClick(i);
      });

  	}

    setBox(measurer, 0, 0, 0.1f, 0.1f);
    setBox(measurer, (numPerRow - 1) * 0.1f , (maxRows -1) * -0.1f, 0.1f, 0.1f);

    auto boundingBox = measurerToBox(measurer);
    //drawDebugBoundingBox(drawTools, boundingBox);
  	return boundingBox;
  },
};

