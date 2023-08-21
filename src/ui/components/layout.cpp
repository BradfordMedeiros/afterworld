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

BoundingBox2D drawLayout(DrawingTools& drawTools, Props& props){
	  	auto layoutPtr = typeFromProps<Layout>(props, layoutSymbol);
	  	modassert(layoutPtr, "layout prop not provided");
	  	auto layout = *layoutPtr;

      auto boundingBoxMeasurer = createMeasurer();
     	BufferedDrawingTools bufferedDrawingTools {};
			createBufferedDrawingTools(bufferedDrawingTools, drawTools);

			float initialXOffset = floatFromProp(props, xoffsetSymbol, 0.f);
			float initialYOffset = floatFromProp(props, yoffsetSymbol, 0.f);
			{
      	float xoffset = initialXOffset;
      	float yoffset = initialYOffset;
      	for (int i = 0; i < layout.children.size(); i++){
					Props childProps {
						.props  = {},
					};

      	  updatePropValue(childProps, xoffsetSymbol, xoffset);
    			updatePropValue(childProps, yoffsetSymbol, yoffset);
    			auto boundingBox = layout.children.at(i).draw(bufferedDrawingTools.drawTools, childProps);
    			
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
    	}

    	
			auto elementsBox = measurerToBox(boundingBoxMeasurer);
      float elementsLeft = elementsBox.x - (elementsBox.width * 0.5f);
      float elementsRight = elementsBox.x + (elementsBox.width * 0.5f);
      float elementsTop = elementsBox.y + (elementsBox.height * 0.5f);
      float elementsBottom = elementsBox.y - (elementsBox.height * 0.5f);

      float padding = layout.padding.has_value() ? layout.padding.value() : 0.f;
      float outerWidth = elementsBox.width + (2 * padding);
      float outerHeight = elementsBox.height + (2 * padding);
      if (layout.minwidth > outerWidth){
      	outerWidth = layout.minwidth;
      }
      if (layout.minheight > outerHeight){
      	outerHeight = layout.minheight;
      }

      float outerLeft = elementsBox.x - (outerWidth * 0.5f);
      float outerRight = elementsBox.x + (outerWidth * 0.5f);
      float outerTop = elementsBox.y + (outerHeight * 0.5f);
      float outerBottom = elementsBox.y - (outerHeight * 0.5f);

   		float outerCenterX = (outerRight + outerLeft) * 0.5f;
      float outerCenterY = (outerTop + outerBottom) * 0.5f;

      // offset x / y makes the box start from the bottom left and up
      float outerOffsetX = initialXOffset - outerCenterX + (outerWidth * 0.5f);
      float outerOffsetY = initialYOffset - outerCenterY + (outerHeight * 0.5f);

      float outerLayoutFlowOffsetX = outerOffsetX;
      float outerLayoutFlowOffsetY = outerOffsetY;

      // position the outer correctly, position the elements in the lower left of the outer box
      if (layout.layoutFlowHorizontal == UILayoutFlowNone2){
      	outerLayoutFlowOffsetX = outerOffsetX - (outerWidth * 0.5f);
      }else if (layout.layoutFlowHorizontal == UILayoutFlowPositive2){
      	outerLayoutFlowOffsetX = outerOffsetX;
      }else if (layout.layoutFlowHorizontal == UILayoutFlowNegative2){
      	outerLayoutFlowOffsetX = outerOffsetX - (outerWidth);
      }

      if (layout.layoutFlowVertical == UILayoutFlowNone2){
      	outerLayoutFlowOffsetY = outerOffsetY - (outerHeight * 0.5f);
      }else if (layout.layoutFlowVertical == UILayoutFlowPositive2){
      	outerLayoutFlowOffsetY = outerOffsetY;
      }else if (layout.layoutFlowVertical == UILayoutFlowNegative2){
      	outerLayoutFlowOffsetY = outerOffsetY - (outerHeight);
      }



      float outerCenterXFinal = outerCenterX + outerLayoutFlowOffsetX;
      float outerCenterYFinal = outerCenterY + outerLayoutFlowOffsetY;
      float newOuterLeft = outerLeft + outerLayoutFlowOffsetX;
      float newOuterRight = outerRight + outerLayoutFlowOffsetX;
      float newOuterBottom = outerBottom + outerLayoutFlowOffsetY;
      float newOuterTop = outerTop + outerLayoutFlowOffsetY;

      float containerElementsAdjustX = newOuterLeft - elementsLeft;
      float containerElementsAdjustY = newOuterBottom - elementsBottom;
      float layoutFlowOffsetX = containerElementsAdjustX;
      float layoutFlowOffsetY = containerElementsAdjustY;

      float newElementsRight = elementsRight + containerElementsAdjustX;
      float newElementsTop = elementsTop + containerElementsAdjustY;
      float newElementsX = elementsBox.x + containerElementsAdjustX;
      float newElementsY = elementsBox.y + containerElementsAdjustY;

      if (layout.alignHorizontal == UILayoutFlowNegative2){
      	layoutFlowOffsetX += padding;
      }else if (layout.alignHorizontal == UILayoutFlowNone2){
      	layoutFlowOffsetX += outerCenterXFinal - newElementsX;
      }else if (layout.alignHorizontal == UILayoutFlowPositive2){
      	float diffToRight = newOuterRight - newElementsRight - padding;
      	layoutFlowOffsetX += diffToRight;
      }

      if (layout.alignVertical == UILayoutFlowNegative2){
      	layoutFlowOffsetY += padding;
      }else if (layout.alignVertical == UILayoutFlowNone2){
      	layoutFlowOffsetY += outerCenterYFinal - newElementsY;
      }else if (layout.alignVertical == UILayoutFlowPositive2){
      	float diffToTop = newOuterTop - newElementsTop - padding;
      	layoutFlowOffsetY += diffToTop;
      }


      if (true){
      	BoundingBox2D outerBounding {
      		.x = outerCenterXFinal,
      		.y = outerCenterYFinal,
      		.width = outerWidth,
      		.height = outerHeight,
      	};
      	drawDebugBoundingBox(drawTools, outerBounding, glm::vec4(0.f, 1.f, 1.f, 1.f));

      	BoundingBox2D innerBounding {
      		.x = elementsBox.x + layoutFlowOffsetX,
      		.y = elementsBox.y + layoutFlowOffsetY,
      		.width = elementsBox.width,
      		.height = elementsBox.height,
      	};
      	drawDebugBoundingBox(drawTools, innerBounding, glm::vec4(1.f, 1.f, 0.f, 1.f));	
      }

      if (true && layout.showBackpanel){
      	drawTools.drawRect(outerCenterXFinal, outerCenterYFinal, outerWidth, outerHeight, false, layout.tint, std::nullopt, true, std::nullopt /* mapping id */, std::nullopt);
      }
    	drawBufferedData(bufferedDrawingTools, glm::vec2(layoutFlowOffsetX, layoutFlowOffsetY));
	  	//BoundingBox2D boundingBox {
	  	//  .x = x,
	  	//  .y = y,
	  	//  .width = width,
	  	//  .height = height,
	  	//};
	  	//if (layout.borderColor.has_value()){
	  	//	drawDebugBoundingBox(drawTools, boundingBox, layout.borderColor);
	  	//}
	  	//return boundingBox;
	  	return BoundingBox2D {
	  		.x = outerCenterXFinal,
	  		.y = outerCenterYFinal,
	  		.width = outerWidth,
	  		.height = outerHeight,
	  	};
}

Component layoutComponent  {
	  .draw = drawLayout,
};

