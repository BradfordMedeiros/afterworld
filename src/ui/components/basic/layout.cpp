#include "./layout.h"

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
	std::optional<float> maxWidth;
  std::optional<ShapeOptions> shaderId;
};
struct BufferedRect {
	int drawOrder;
  float centerX;
  float centerY;
  float width;
  float height;
  bool perma;
  std::optional<glm::vec4> tint;
  bool ndi;
  std::optional<objid> selectionId;
  std::optional<std::string> texture;
  std::optional<ShapeOptions> shaderId;
  std::optional<objid> trackingId;
};
struct BufferedLine2D {
	int drawOrder;
  glm::vec3 fromPos;
  glm::vec3 toPos;
  bool perma;
  std::optional<glm::vec4> tint;
  bool ndi;
  std::optional<objid> selectionId;
  std::optional<std::string> texture;
  std::optional<ShapeOptions> shaderId;
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
  drawTools.drawText = [&bufferedDrawingTools](std::string word, float left, float top, unsigned int fontSize, bool permatext, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<std::string> fontFamily, std::optional<objid> selectionId, std::optional<float> maxWidth, std::optional<ShapeOptions> shaderId) -> void {
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
			.maxWidth = maxWidth,
			.shaderId = shaderId,
  	});
  };
  drawTools.getTextDimensionsNdi = realTools.getTextDimensionsNdi;
  drawTools.drawRect = [&bufferedDrawingTools](float centerX, float centerY, float width, float height, bool perma, std::optional<glm::vec4> tint, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture, std::optional<ShapeOptions> shaderId, std::optional<objid> trackingId) -> void {
  	bufferedDrawingTools.bufferedData.bufferedRect.push_back(BufferedRect{
 			.drawOrder = bufferedDrawingTools.bufferedData.bufferedIndex++,
  		.centerX = centerX,
  		.centerY = centerY,
  		.width = width,
  		.height = height,
  		.perma = perma,
  		.tint = tint,
  		.ndi = ndi,
  		.selectionId = selectionId,
  		.texture = texture,
  		.shaderId = shaderId,
  		.trackingId = trackingId,
  	});  	
  };
  drawTools.drawLine2D = [&bufferedDrawingTools](glm::vec3 fromPos, glm::vec3 toPos, bool perma, std::optional<glm::vec4> tint, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture, std::optional<ShapeOptions> shaderId) -> void {
  	bufferedDrawingTools.bufferedData.buffered2DLines.push_back(BufferedLine2D{
  		.drawOrder = bufferedDrawingTools.bufferedData.bufferedIndex++,
  		.fromPos = fromPos,
  		.toPos = toPos,
  		.perma = perma,
  		.tint = tint,
  		.ndi = ndi,
  		.selectionId = selectionId,
  		.texture = texture,
  		.shaderId = shaderId,
  	});
  };
  drawTools.registerCallbackFns = realTools.registerCallbackFns;
  drawTools.registerCallbackFnsHandler = realTools.registerCallbackFnsHandler;
  drawTools.registerCallbackRightFns = realTools.registerCallbackRightFns;
  drawTools.registerInputFns = realTools.registerInputFns;
  drawTools.registerAutoFocus = realTools.registerAutoFocus;
  drawTools.selectedId = realTools.selectedId;
  drawTools.focusedId = realTools.focusedId;
  drawTools.getClipboardString = realTools.getClipboardString;
  drawTools.setClipboardString = realTools.setClipboardString;

	bufferedDrawingTools.drawTools = drawTools,
	bufferedDrawingTools.realTools = &realTools,
	bufferedDrawingTools.bufferedData = BufferedData {
		.bufferedIndex = 0,
		.bufferedText = {},
		.bufferedRect = {},
		.buffered2DLines = {},
	};
}

void drawBufferedData(BufferedDrawingTools& bufferedTools, glm::vec2 positionOffset, std::optional<ShapeOptions> shapeOptions){
	int bufferedTextIndex = 0;
	int bufferedRectIndex = 0;
	int bufferedLine2DIndex = 0;
	for (int i = 0; i < bufferedTools.bufferedData.bufferedIndex; i++){
		if (bufferedTools.bufferedData.bufferedText.size() > bufferedTextIndex && bufferedTools.bufferedData.bufferedText.at(bufferedTextIndex).drawOrder == i){
			BufferedText& bufferedText = bufferedTools.bufferedData.bufferedText.at(bufferedTextIndex);
			bufferedTools.realTools -> drawText(bufferedText.word, bufferedText.left + positionOffset.x, bufferedText.top + positionOffset.y, bufferedText.fontSize, bufferedText.permatext, bufferedText.tint, bufferedText.textureId, bufferedText.ndi, bufferedText.fontFamily, bufferedText.selectionId, bufferedText.maxWidth, shapeOptions.has_value() ? shapeOptions.value() : bufferedText.shaderId);
			bufferedTextIndex++;
		}else if(bufferedTools.bufferedData.bufferedRect.size() > bufferedRectIndex && bufferedTools.bufferedData.bufferedRect.at(bufferedRectIndex).drawOrder == i){
			BufferedRect& bufferedRect = bufferedTools.bufferedData.bufferedRect.at(bufferedRectIndex);
			bufferedTools.realTools -> drawRect(bufferedRect.centerX + positionOffset.x, bufferedRect.centerY + positionOffset.y, bufferedRect.width, bufferedRect.height, bufferedRect.perma, bufferedRect.tint, bufferedRect.ndi, bufferedRect.selectionId, bufferedRect.texture, shapeOptions.has_value() ? shapeOptions.value() : bufferedRect.shaderId, bufferedRect.trackingId);
			bufferedRectIndex++;
		}else if (bufferedTools.bufferedData.buffered2DLines.size() > bufferedLine2DIndex && bufferedTools.bufferedData.buffered2DLines.at(bufferedLine2DIndex).drawOrder == i){
			BufferedLine2D bufferedLine2D = bufferedTools.bufferedData.buffered2DLines.at(bufferedLine2DIndex);
			bufferedTools.realTools -> drawLine2D(bufferedLine2D.fromPos + glm::vec3(positionOffset.x, positionOffset.y, 0.f), bufferedLine2D.toPos + glm::vec3(positionOffset.x, positionOffset.y, 0.f), bufferedLine2D.perma, bufferedLine2D.tint, bufferedLine2D.ndi, bufferedLine2D.selectionId, bufferedLine2D.texture, shapeOptions.has_value() ? shapeOptions.value() : bufferedLine2D.shaderId);
			bufferedLine2DIndex++;
		}
	}
}

BoundingBox2D repositionLastElement(BufferedDrawingTools& bufferedDrawingTools, BoundingBox2D& boundingBox, float xoffset, float yoffset, int startDrawIndex, int endDrawIndex){
	modassert(startDrawIndex <= endDrawIndex, "invalid indexs for reposition");
	auto sides = calculateSides(boundingBox);
	//std::cout << "layout : xoffset = " << xoffset << ", yoffset = " << yoffset << std::endl;
	//std::cout << "layout: element: " << print(boundingBox) << std::endl;
	//std::cout << "layout: sides: " << print(sides) << std::endl;
	//std::cout << "layout: need to offset: xoffset = " << (xoffset - sides.left) << ", yoffset = " << (yoffset - sides.bottom) << std::endl;
	// reposition to xoffset
	//

	float amountToOffsetX = xoffset - sides.left;
	boundingBox.x += amountToOffsetX;

	float amountToOffsetY = yoffset - sides.top;
	boundingBox.y += amountToOffsetY;
 	//drawDebugBoundingBox(*(bufferedDrawingTools.realTools), boundingBox, glm::vec4(0.f, 1.f, 1.f, 1.f));

	for (int i = bufferedDrawingTools.bufferedData.bufferedRect.size() - 1; i >= 0; i--){
		BufferedRect& bufferedRect = bufferedDrawingTools.bufferedData.bufferedRect.at(i);
		if (bufferedRect.drawOrder < startDrawIndex){
			break;
		}
		if (bufferedRect.drawOrder < endDrawIndex){
			bufferedRect.centerX += amountToOffsetX;
			bufferedRect.centerY += amountToOffsetY;
		}
	}
	for (int i = bufferedDrawingTools.bufferedData.bufferedText.size() - 1; i >= 0; i--){
		BufferedText& bufferedText = bufferedDrawingTools.bufferedData.bufferedText.at(i);
		if (bufferedText.drawOrder < startDrawIndex){
			break;
		}
		if (bufferedText.drawOrder < endDrawIndex){
			bufferedText.left += amountToOffsetX;
			bufferedText.top += amountToOffsetY;
		}
	}

	for (int i = bufferedDrawingTools.bufferedData.buffered2DLines.size() - 1; i >= 0; i--){
		BufferedLine2D& buffered2DLines = bufferedDrawingTools.bufferedData.buffered2DLines.at(i);
		if (buffered2DLines.drawOrder < startDrawIndex){
			break;
		}
		if (buffered2DLines.drawOrder < endDrawIndex){
			buffered2DLines.fromPos.x += amountToOffsetX;
			buffered2DLines.fromPos.y += amountToOffsetY;
			buffered2DLines.toPos.x += amountToOffsetX;
			buffered2DLines.toPos.y += amountToOffsetY;
		}
	}

	return boundingBox;
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


    int startDrawIndex  = bufferedDrawingTools.bufferedData.bufferedIndex;
    auto boundingBox = layout.children.at(i).draw(bufferedDrawingTools.drawTools, childProps);
    int upperDrawing = bufferedDrawingTools.bufferedData.bufferedIndex;
   	boundingBox = repositionLastElement(bufferedDrawingTools, boundingBox, xoffset, yoffset, startDrawIndex, upperDrawing);
    //std::cout << print(boundingBox) << std::endl;
    measureBoundingBox(boundingBoxMeasurer, boundingBox);

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

 	modassert(layout.children.size() > 0, "layout must have at least 1 element");
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

  float layoutFlowOffsetX = 0.f;
  if (layout.alignHorizontal == UILayoutFlowNegative2){
  	layoutFlowOffsetX = finalOuterSides.left - elementSides.left + padding;
  }else if (layout.alignHorizontal == UILayoutFlowNone2){
  	layoutFlowOffsetX = outerBoundingBox.x - elementsBox.x;
  }else if (layout.alignHorizontal == UILayoutFlowPositive2){
  	layoutFlowOffsetX = finalOuterSides.right - elementSides.right - padding;
  }

  float layoutFlowOffsetY = 0.f;
  if (layout.alignVertical == UILayoutFlowNegative2){
  	layoutFlowOffsetY = finalOuterSides.bottom - elementSides.bottom + padding;
  }else if (layout.alignVertical == UILayoutFlowNone2){
  	layoutFlowOffsetY = outerBoundingBox.y - elementsBox.y;
  }else if (layout.alignVertical == UILayoutFlowPositive2){
  	layoutFlowOffsetY = finalOuterSides.top - elementSides.top - padding;
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
  float padding = layout.padding.has_value() ? layout.padding.value() : 0.f;

  BufferedDrawingTools bufferedDrawingTools {};
	createBufferedDrawingTools(bufferedDrawingTools, drawTools);

  // Draws the elements into the buffer with their relative spacing
	auto elementsBox = drawLayoutElements(bufferedDrawingTools, layout);

	// Figures out where the backpanel will go based upon the elements sizing, and layout properties
	auto outerBox = calculateAdjustedBackpanel(elementsBox, layout, padding, floatFromProp(props, xoffsetSymbol, 0.f), floatFromProp(props, yoffsetSymbol, 0.f));

	// Repositions elements into the backpanel, taking into account alignment
	auto flowOffsets = calculateFlowOffsets(layout, elementsBox, outerBox, padding);
	//auto flowOffsets = glm::vec2(0.f, 0.f);

	if (layout.borderColor.has_value()){
	 	drawDebugBoundingBox(drawTools, outerBox, layout.borderColor, layout.shapeOptions);
	}

  if (false){
  	BoundingBox2D innerBounding {
  		.x = elementsBox.x + flowOffsets.x,
  		.y = elementsBox.y + flowOffsets.y,
  		.width = elementsBox.width,
  		.height = elementsBox.height,
  	};
  	//drawDebugBoundingBox(drawTools, innerBounding, glm::vec4(1.f, 1.f, 0.f, 1.f));	
  }

  if (layout.showBackpanel){
   	drawTools.drawRect(outerBox.x, outerBox.y, outerBox.width, outerBox.height, false, layout.tint, true, std::nullopt /* mapping id */, std::nullopt, layout.shapeOptions, std::nullopt);
  }
 	drawBufferedData(bufferedDrawingTools, glm::vec2(flowOffsets.x, flowOffsets.y), layout.shapeOptions);
 	return outerBox;
}


Component layoutComponent  {
	  .draw = drawLayout,
};


AlignmentParams defaultAlignment {
  .layoutFlowHorizontal = UILayoutFlowNone2,
  .layoutFlowVertical = UILayoutFlowNone2,
};
Component simpleVerticalLayout(std::vector<Component>& children, glm::vec2 minDim, AlignmentParams defaultAlignment, glm::vec4 borderColor, float padding, glm::vec4 tint, std::optional<ShapeOptions> shapeOptions){
  Layout layout {
    .tint = tint,
    .showBackpanel = true,
    .borderColor = borderColor,
    .minwidth = minDim.x,
    .minheight = minDim.y,
    .layoutType = LAYOUT_VERTICAL2,
    .layoutFlowHorizontal = defaultAlignment.layoutFlowHorizontal,
    .layoutFlowVertical = defaultAlignment.layoutFlowVertical,
    .alignHorizontal = UILayoutFlowNegative2,
    .alignVertical = UILayoutFlowNone2,
    .spacing = 0.f,
    .minspacing = 0.f,
    .padding = padding,
    .shapeOptions = shapeOptions,
    .children = children,
  };
  Props listLayoutProps {
    .props = {
      { .symbol = layoutSymbol, .value = layout },
    },
  };
  return withPropsCopy(layoutComponent, listLayoutProps);
}

Component simpleHorizontalLayout(std::vector<Component>& children, float padding, glm::vec4 tint){
  Layout layout {
    .tint = tint,
    .showBackpanel = true,
    .borderColor = glm::vec4(1.f, 0.f, 0.f, 0.f),
    .minwidth = 0.f,
    .minheight = 0.f,
    .layoutType = LAYOUT_HORIZONTAL2,
    .layoutFlowHorizontal = UILayoutFlowNone2,
    .layoutFlowVertical = UILayoutFlowNone2,
    .alignHorizontal = UILayoutFlowNone2,
    .alignVertical = UILayoutFlowNone2,
    .spacing = 0.01f,
    .minspacing = 0.f,
    .padding = padding,
    .children = children,
  };
  Props listLayoutProps {
    .props = {
      { .symbol = layoutSymbol, .value = layout },
    },
  };
  return withPropsCopy(layoutComponent, listLayoutProps);
}

Component simpleLayout(Component& component, glm::vec2 minDim, AlignmentParams defaultAlignment, glm::vec4 borderColor, float padding){
	std::vector<Component> components = { component };
  return simpleVerticalLayout(components, minDim, defaultAlignment, borderColor, padding);
}