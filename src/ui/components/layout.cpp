#include "./layout.h"
extern CustomApiBindings* gameapi;


// The buffered approach could be supported by the game engine more easily, but doing here for now as to not pollute the engine code
struct BufferedText {
	int drawOrder;
	std::string word;
	float left;
	float top;
	unsigned int fontSize;
	bool permatext;
	std::optional<glm::vec4> tint;
	std::optional<unsigned int> textureId;
	bool ndi;
	std::optional<std::string> fontFamily;
	std::optional<objid> selectionId;
};
struct BufferedRect {
	int drawOrder;
  float centerX;
  float centerY;
  float width;
  float height;
  bool perma;
  std::optional<glm::vec4> tint;
  std::optional<unsigned int> textureId;
  bool ndi;
  std::optional<objid> selectionId;
  std::optional<std::string> texture;
};
struct BufferedLine2D {
	int drawOrder;
  glm::vec3 fromPos;
  glm::vec3 toPos;
  bool perma;
  std::optional<glm::vec4> tint;
  std::optional<unsigned int> textureId;
  bool ndi;
  std::optional<objid> selectionId;
  std::optional<std::string> texture;
};
struct BufferedData {
	int bufferedIndex;
	std::vector<BufferedText> bufferedText;
	std::vector<BufferedRect> bufferedRect;
	std::vector<BufferedLine2D> buffered2DLines;
};
struct BufferedDrawingTools {
	DrawingTools drawTools;
	DrawingTools* realTools;
	BufferedData bufferedData;
};
void createBufferedDrawingTools(BufferedDrawingTools& bufferedDrawingTools, DrawingTools& realTools){
	DrawingTools drawTools {};
  drawTools.drawText = [&bufferedDrawingTools](std::string word, float left, float top, unsigned int fontSize, bool permatext, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<std::string> fontFamily, std::optional<objid> selectionId) -> void {
  	bufferedDrawingTools.bufferedData.bufferedText.push_back(BufferedText{
 			.drawOrder = bufferedDrawingTools.bufferedData.bufferedIndex++,
 			.word = word,
			.left = left,
			.top = top,
			.fontSize = fontSize,
			.permatext = permatext,
			.tint = tint,
			.textureId = textureId,
			.ndi = ndi,
			.fontFamily = fontFamily,
			.selectionId = selectionId,
  	});
  };
  drawTools.drawRect = [&bufferedDrawingTools](float centerX, float centerY, float width, float height, bool perma, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture) -> void {
  	bufferedDrawingTools.bufferedData.bufferedRect.push_back(BufferedRect{
 			.drawOrder = bufferedDrawingTools.bufferedData.bufferedIndex++,
  		.centerX = centerX,
  		.centerY = centerY,
  		.width = width,
  		.height = height,
  		.perma = perma,
  		.tint = tint,
  		.textureId = textureId,
  		.ndi = ndi,
  		.selectionId = selectionId,
  		.texture = texture,
  	});  	
  };
  drawTools.drawLine2D = [&bufferedDrawingTools](glm::vec3 fromPos, glm::vec3 toPos, bool perma, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture) -> void {
  	bufferedDrawingTools.bufferedData.buffered2DLines.push_back(BufferedLine2D{
  		.drawOrder = bufferedDrawingTools.bufferedData.bufferedIndex++,
  		.fromPos = fromPos,
  		.toPos = toPos,
  		.perma = perma,
  		.tint = tint,
  		.textureId = textureId,
  		.ndi = ndi,
  		.selectionId = selectionId,
  		.texture = texture,
  	});
  };
  drawTools.registerCallbackFns = realTools.registerCallbackFns;
  drawTools.selectedId = realTools.selectedId;

	bufferedDrawingTools.drawTools = drawTools,
	bufferedDrawingTools.realTools = &realTools,
	bufferedDrawingTools.bufferedData = BufferedData {
		.bufferedIndex = 0,
		.bufferedText = {},
		.bufferedRect = {},
		.buffered2DLines = {},
	};
}

void drawBufferedData(BufferedDrawingTools& bufferedTools, glm::vec2 positionOffset){
	int bufferedTextIndex = 0;
	int bufferedRectIndex = 0;
	int bufferedLine2DIndex = 0;
	for (int i = 0; i < bufferedTools.bufferedData.bufferedIndex; i++){
		if (bufferedTools.bufferedData.bufferedText.size() > bufferedTextIndex && bufferedTools.bufferedData.bufferedText.at(bufferedTextIndex).drawOrder == i){
			BufferedText& bufferedText = bufferedTools.bufferedData.bufferedText.at(bufferedTextIndex);
			bufferedTools.realTools -> drawText(bufferedText.word, bufferedText.left + positionOffset.x, bufferedText.top + positionOffset.y, bufferedText.fontSize, bufferedText.permatext, bufferedText.tint, bufferedText.textureId, bufferedText.ndi, bufferedText.fontFamily, bufferedText.selectionId);
			bufferedTextIndex++;
		}else if(bufferedTools.bufferedData.bufferedRect.size() > bufferedRectIndex && bufferedTools.bufferedData.bufferedRect.at(bufferedRectIndex).drawOrder == i){
			BufferedRect& bufferedRect = bufferedTools.bufferedData.bufferedRect.at(bufferedRectIndex);
			bufferedTools.realTools -> drawRect(bufferedRect.centerX + positionOffset.x, bufferedRect.centerY + positionOffset.y, bufferedRect.width, bufferedRect.height, bufferedRect.perma, bufferedRect.tint, bufferedRect.textureId, bufferedRect.ndi, bufferedRect.selectionId, bufferedRect.texture);
			bufferedRectIndex++;
		}else if (bufferedTools.bufferedData.buffered2DLines.size() > bufferedLine2DIndex && bufferedTools.bufferedData.buffered2DLines.at(bufferedLine2DIndex).drawOrder == i){
			BufferedLine2D bufferedLine2D = bufferedTools.bufferedData.buffered2DLines.at(bufferedLine2DIndex);
			bufferedTools.realTools -> drawLine2D(bufferedLine2D.fromPos + glm::vec3(positionOffset.x, positionOffset.y, 0.f), bufferedLine2D.toPos + glm::vec3(positionOffset.x, positionOffset.y, 0.f), bufferedLine2D.perma, bufferedLine2D.tint, bufferedLine2D.textureId, bufferedLine2D.ndi, bufferedLine2D.selectionId, bufferedLine2D.texture);
			bufferedLine2DIndex++;
		}
	}
}

BoundingBox2D drawLayoutElements(BufferedDrawingTools& bufferedDrawingTools, Layout& layout){
  auto boundingBoxMeasurer = createMeasurer();

  float xoffset = 0.f;
  float yoffset = 0.f;
  for (int i = 0; i < layout.children.size(); i++){
		Props childProps {
			.props  = {},
		};
    updatePropValue(childProps, xoffsetSymbol, xoffset);
    updatePropValue(childProps, yoffsetSymbol, yoffset);

    auto boundingBox = layout.children.at(i).draw(bufferedDrawingTools.drawTools, childProps);
    //repositionLastDraw(bufferedDrawingTools, calcPositionFromBoundingBox(boundingBox));

    setX(boundingBoxMeasurer, boundingBox.x + (boundingBox.width * 0.5f));
    setX(boundingBoxMeasurer, boundingBox.x - (boundingBox.width * 0.5f));
    setY(boundingBoxMeasurer, boundingBox.y + (boundingBox.height * 0.5f));
    setY(boundingBoxMeasurer, boundingBox.y - (boundingBox.height * 0.5f));
    
    if (layout.layoutType == LAYOUT_VERTICAL2){
			float spacingPerItemHeight = boundingBox.height + layout.spacing;
			if (spacingPerItemHeight < layout.minspacing){
				spacingPerItemHeight = layout.minspacing;
			}
    	yoffset -= spacingPerItemHeight;
    }else{
    	float spacingPerItemWidth = boundingBox.width + layout.spacing;
    	if (spacingPerItemWidth < layout.minspacing){
    		spacingPerItemWidth = layout.minspacing;
    	}
    	xoffset += spacingPerItemWidth;
    }
  }
	return measurerToBox(boundingBoxMeasurer);

}

BoundingBox2D calculateAdjustedBackpanel(BoundingBox2D& elementsBox, Layout& layout, float padding, float initialXOffset, float initialYOffset){  
  float outerWidth = elementsBox.width + (2 * padding);
  float outerHeight = elementsBox.height + (2 * padding);
  if (layout.minwidth > outerWidth){
  	outerWidth = layout.minwidth;
  }
  if (layout.minheight > outerHeight){
  	outerHeight = layout.minheight;
  }

  BoundingBox2D outerBox {
  	.x = elementsBox.x,
  	.y = elementsBox.y,
  	.width = outerWidth,
  	.height = outerHeight,
  };

  auto outerSides = calculateSides(outerBox);
  float outerLayoutFlowOffsetX = 0.f;
  float outerLayoutFlowOffsetY = 0.f;
	
  // position the outer correctly, position the elements in the lower left of the outer box
  if (layout.layoutFlowHorizontal == UILayoutFlowPositive2){
  	outerLayoutFlowOffsetX = 0.5f * outerWidth;
  }else if (layout.layoutFlowHorizontal == UILayoutFlowNegative2){
  	outerLayoutFlowOffsetX = -0.5f * outerWidth;
  }
  if (layout.layoutFlowVertical == UILayoutFlowPositive2){
  	outerLayoutFlowOffsetY = 0.5f * outerHeight;
  }else if (layout.layoutFlowVertical == UILayoutFlowNegative2){
  	outerLayoutFlowOffsetY = -0.5f * outerHeight;
  }

 	BoundingBox2D outerBoundingBox {
		.x = initialXOffset + outerLayoutFlowOffsetX,
		.y = initialYOffset + outerLayoutFlowOffsetY,
		.width = outerWidth,
		.height = outerHeight,
 	};

	return outerBoundingBox;
}

struct FlowOffsets {
	float x;
	float y;
};

FlowOffsets calculateFlowOffsets(Layout& layout, BoundingBox2D& elementsBox, BoundingBox2D& outerBoundingBox, float padding){
 	auto finalOuterSides = calculateSides(outerBoundingBox);
	auto elementSides = calculateSides(elementsBox);
  float containerElementsAdjustX = finalOuterSides.left - elementSides.left;
  float containerElementsAdjustY = finalOuterSides.bottom - elementSides.bottom;

  float layoutFlowOffsetX = containerElementsAdjustX;
  float layoutFlowOffsetY = containerElementsAdjustY;

  float newElementsRight = elementSides.right + containerElementsAdjustX;
  float newElementsTop = elementSides.top + containerElementsAdjustY;

  float newElementsX = elementsBox.x + containerElementsAdjustX;
  float newElementsY = elementsBox.y + containerElementsAdjustY;

  if (layout.alignHorizontal == UILayoutFlowNegative2){
  	layoutFlowOffsetX += padding;
  }else if (layout.alignHorizontal == UILayoutFlowNone2){
  	layoutFlowOffsetX += outerBoundingBox.x - newElementsX;
  }else if (layout.alignHorizontal == UILayoutFlowPositive2){
  	float diffToRight = finalOuterSides.right - newElementsRight - padding;
  	layoutFlowOffsetX += diffToRight;
  }

  if (layout.alignVertical == UILayoutFlowNegative2){
  	layoutFlowOffsetY += padding;
  }else if (layout.alignVertical == UILayoutFlowNone2){
  	layoutFlowOffsetY += outerBoundingBox.y - newElementsY;
  }else if (layout.alignVertical == UILayoutFlowPositive2){
  	float diffToTop = finalOuterSides.top - newElementsTop - padding;
  	layoutFlowOffsetY += diffToTop;
  }

  return FlowOffsets {
  	.x = layoutFlowOffsetX,
  	.y = layoutFlowOffsetY,
  };
}


BoundingBox2D drawLayout(DrawingTools& drawTools, Props& props){
	auto layoutPtr = typeFromProps<Layout>(props, layoutSymbol);
	modassert(layoutPtr, "layout prop not provided");
	auto layout = *layoutPtr;

  BufferedDrawingTools bufferedDrawingTools {};
	createBufferedDrawingTools(bufferedDrawingTools, drawTools);

	// Draw the elements
	// initial x,y offset shouldn't actually matter.  
  float padding = layout.padding.has_value() ? layout.padding.value() : 0.f;
	auto elementsBox = drawLayoutElements(bufferedDrawingTools, layout);
	auto outerBox = calculateAdjustedBackpanel(elementsBox, layout, padding, floatFromProp(props, xoffsetSymbol, 0.f), floatFromProp(props, yoffsetSymbol, 0.f));
	auto flowOffsets = calculateFlowOffsets(layout, elementsBox, outerBox, padding);

  if (true){
  	drawDebugBoundingBox(drawTools, outerBox, glm::vec4(0.f, 1.f, 1.f, 1.f));
  	BoundingBox2D innerBounding {
  		.x = elementsBox.x + flowOffsets.x,
  		.y = elementsBox.y + flowOffsets.y,
  		.width = elementsBox.width,
  		.height = elementsBox.height,
  	};
  	//drawDebugBoundingBox(drawTools, innerBounding, glm::vec4(1.f, 1.f, 0.f, 1.f));	
  }

  if (true && layout.showBackpanel){
   	drawTools.drawRect(outerBox.x, outerBox.y, outerBox.width, outerBox.height, false, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt, true, std::nullopt /* mapping id */, std::nullopt);
  }
 	drawBufferedData(bufferedDrawingTools, glm::vec2(flowOffsets.x, flowOffsets.y));
 	return outerBox;
}


Component layoutComponent  {
	  .draw = drawLayout,
};

