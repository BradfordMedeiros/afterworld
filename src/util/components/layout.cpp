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

Component createLayoutComponent(Layout& layout){
	Component layoutComponent  {
	  .draw = [&layout](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
	  	float x = 0.f;
	  	float y = 0.f;
      float width = 0.f;
      float height = 0.f;
      if (layout.minwidth > width){
      	width = layout.minwidth;
      }
      if (layout.minheight > height){
      	height = layout.minheight;
      }
      if (layout.showBackpanel){
      	drawTools.drawRect(x, y, width, height, false, layout.tint, std::nullopt, true, std::nullopt /* mapping id */, std::nullopt);
      }

      float xoffset = 0.f;
      float yoffset = 0.f;
      props.style.xoffset = 0.f;
      props.style.yoffset = 0.f;
      props.additionalYOffset = 0.f;

			if (layout.margin.has_value()){
				yoffset -= layout.margin.value();
    		xoffset += layout.margin.value();
			}					
     
     auto boundingBoxMeasurer = createMeasurer();
     	BufferedDrawingTools bufferedDrawingTools {};
			createBufferedDrawingTools(bufferedDrawingTools, drawTools);
      for (int i = 0; i < layout.children.size(); i++){
    		props.style.yoffset = yoffset;	
    		props.style.xoffset = xoffset;
    		auto boundingBox = layout.children.at(i).draw(bufferedDrawingTools.drawTools, props);
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

			auto layoutOriginalBox = measurerToBox(boundingBoxMeasurer);


    	float layoutFlowOffsetX = 0.f;
    	float layoutFlowOffsetY = 0.f;

    	float fullLeft = x - (width * 0.5f);
    	float fullRight = x + (width * 0.5f);
    	float fullTop = y + (height * 0.5f);
    	float fullBottom = y - (height * 0.5f);
    	float elementsLeft = layoutOriginalBox.x - (layoutOriginalBox.width * 0.5f);
    	float elementsRight = layoutOriginalBox.x + (layoutOriginalBox.width * 0.5f);
    	float elementsTop = layoutOriginalBox.y + (layoutOriginalBox.height * 0.5f);
    	float elementsBottom = layoutOriginalBox.y - (layoutOriginalBox.height * 0.5f);

    	if (layout.layoutFlowHorizontal == UILayoutFlowNegative2){
    		float diffLeft = fullLeft - elementsLeft;
    		layoutFlowOffsetX = diffLeft;
    	}else if (layout.layoutFlowHorizontal == UILayoutFlowPositive2){
    		float diffRight = fullRight - elementsRight;
    		layoutFlowOffsetX = diffRight;
    	}
    	if (layout.layoutFlowVertical == UILayoutFlowNegative2){
    		float diffBottom = fullBottom - elementsBottom;
    		layoutFlowOffsetY = diffBottom;
    	}else if (layout.layoutFlowVertical == UILayoutFlowPositive2){
    		float diffTop = fullTop - elementsTop;
    		layoutFlowOffsetY = diffTop;
    	}
    	drawBufferedData(bufferedDrawingTools, glm::vec2(layoutFlowOffsetX, layoutFlowOffsetY));

	  	BoundingBox2D boundingBox {
	  	  .x = x,
	  	  .y = y,
	  	  .width = width,
	  	  .height = height,
	  	};
	  	drawDebugBoundingBox(drawTools, boundingBox, layout.borderColor);
	  	return boundingBox;
	  },
	  .imMouseSelect = [](std::optional<objid> mappingIdSelected) -> void {
	     
	  }  
	};
	return layoutComponent;
}

