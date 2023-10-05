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

    int numColumns = 0;
    int numRows = 0;

    auto setFixedSize = boolFromProp(props, fixedSizeSymbol, true);
  	modassert(imageList, "invalid image list prop");

    float size = floatFromProp(props, sizeSymbol, 0.1f);
    float width = size;
    float height = size;

  	for (int i = offset; i < imageList -> images.size(); i++){
      auto mappingId = uniqueMenuItemMappingId();
  		bool selected = drawTools.selectedId.has_value() && drawTools.selectedId.has_value() && (drawTools.selectedId.value() == mappingId);
    	int column = (i - offset) % numPerRow;
    	int row = (i - offset) / numPerRow;
      if (row >= maxRows){
        break;
      }

      if (row > numRows){
        numRows = row;
      }
      if (column > numColumns){
        numColumns = column;
      }

      float x = column * width;
      float y = -1.f * row * height;

      auto image = imageList -> images.at(i);
      auto tint = selected ? glm::vec4(2.f, 2.f, 2.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f);
      if (image.tint.has_value()){
        tint = image.tint.value();
      }
    	drawTools.drawRect(x, y, width, height, false, tint, std::nullopt, true, mappingId, image.image, std::nullopt);
      BoundingBox2D box {
        .x = x,
        .y = y,
        .width  = width,
        .height = height,
      };
      if (!image.tint.has_value() && selected){
        drawDebugBoundingBox(drawTools, box, glm::vec4(1.f, 1.f, 1.f, 1.f));
      }
      drawTools.registerCallbackFns(mappingId, [onClick, i]() -> void {
        onClick(i);
      });

  	}

    setBox(measurer, 0, 0, width, height);

    if (setFixedSize){
      setBox(measurer, (numPerRow - 1) * width , (maxRows -1) * -1 * height, width, height);
    }else{
      setBox(measurer, numColumns * width, numRows * -1 * height, width, height);
    }

    auto boundingBox = measurerToBox(measurer);
    //drawDebugBoundingBox(drawTools, boundingBox);
  	return boundingBox;
  },
};

