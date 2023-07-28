#include "./layout.h"

extern CustomApiBindings* gameapi;

/*
enum UILayoutFlowType { UILayoutFlowNone, UILayoutFlowNegative, UILayoutFlowPositive };
struct LayoutMargin {
  float margin;
  float marginLeft;
  float marginRight;
  float marginBottom;
  float marginTop;
  bool marginSpecified;
  bool marginLeftSpecified;
  bool marginRightSpecified;
  bool marginBottomSpecified;
  bool marginTopSpecified;
};
struct LayoutAnchor {
  std::string target;
  glm::vec3 offset;
  UILayoutFlowType horizontal;
  UILayoutFlowType vertical;
};
struct LayoutBorder {
  float borderSize;
  glm::vec4 borderColor;
  bool hasBorder;
};
enum LayoutContentAlignmentType { LayoutContentAlignment_Negative, LayoutContentAlignment_Neutral, LayoutContentAlignment_Positive };
struct LayoutContentAlignment {
  LayoutContentAlignmentType vertical;
  LayoutContentAlignmentType horizontal;
};
enum LayoutContentSpacing  { LayoutContentSpacing_Pack, LayoutContentSpacing_SpaceForFirst, LayoutContentSpacing_SpaceForLast };
struct GameObjectUILayout {
  float spacing;
  float minSpacing;
  std::vector<std::string> elements;
  BoundInfo boundInfo;
  glm::vec3 panelDisplayOffset;

  LayoutMargin marginValues;
  LayoutAnchor anchor;
  TextureInformation texture;
  float minwidth;
  float minheight;
  int limit;
  float limitsize;

  LayoutContentAlignment alignment;
  LayoutContentAlignmentType contentAlign;
  LayoutContentSpacing contentSpacing;
};*/


//   gameapi -> drawRect(rectX, rectY, rectWidth, rectHeight, false, style.tint, std::nullopt, true, menuItem.mappingId, std::nullopt);
/*
struct Layout {
	glm::vec4 tint;
	bool showBackpanel;
	std::optional<int> borderSize;
};
*/

Component createLayoutComponent(Layout& layout){
	Component layoutComponent  {
	  .draw = [&layout](Props& props) -> BoundingBox2D {
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
      	gameapi -> drawRect(x, y, width, height, false, layout.tint, std::nullopt, true, std::nullopt /* mapping id */, std::nullopt);
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
     

      for (int i = 0; i < layout.children.size(); i++){
    		props.style.yoffset = yoffset;	
    		props.style.xoffset = xoffset;
    		auto boundingBox = layout.children.at(i).draw(props);

    		if (layout.layoutType == LAYOUT_VERTICAL){
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



    		/*lastWidth = boundingBox.width;
    		lastHeight = boundingBox.height;
    		float bottomY = boundingBox.y - (boundingBox.height * 0.5f);
    		float topY = boundingBox.y + (boundingBox.height * 0.5f);
    		float leftX = boundingBox.x - (boundingBox.width * 0.5f);
    		float rightX = boundingBox.x + (boundingBox.width * 0.5f);
	    		if (!minX.has_value()){
	    		  minX = leftX;
	    		}
	    		if (leftX < minX.value()){
	    		  minX = leftX;
	    		}
	    		if (!maxX.has_value()){
	    		  maxX = rightX;
	    		}
	    		if (rightX > maxX.value()){
	    		  maxX = rightX;
	    		}
	    		if (!minY.has_value()){
	    		  minY = bottomY;
	    		}
	    		if (bottomY < minY.value()){
	    		  minY = bottomY;
	    		}
	    		if (!maxY.has_value()){
	    		  maxY = topY;
	    		}
	    		if (topY > maxY.value()){
	    		  maxY = topY;
    		}*/
    	}


	  	BoundingBox2D boundingBox {
	  	  .x = x,
	  	  .y = y,
	  	  .width = width,
	  	  .height = height,
	  	};
	  	drawDebugBoundingBox(boundingBox, layout.borderColor);
	  	return boundingBox;
	  },
	  .imMouseSelect = [](std::optional<objid> mappingIdSelected) -> void {
	     
	  }  
	};
	return layoutComponent;
}

