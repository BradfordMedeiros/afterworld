#include "./slider.h"

extern CustomApiBindings* gameapi;

Slider slider {
  .min  = 0.f,
  .max = 10.f,
  .percentage = 0.f,
  .mappingId = 34545
};
bool update = false;

void drawRight(float x, float y, float width, float height, glm::vec4 color){
  gameapi -> drawRect(x + (width * 0.5f), y, width, height, false, color, std::nullopt, true, slider.mappingId /*radioButton.mappingId */, std::nullopt);
}

Component sliderSelector  {
  .draw = [](Props& props) -> BoundingBox2D {
    float width = 0.2f;
    float height = 0.05f;
    float x = props.style.xoffset + (width * 0.5f);
    float y = props.additionalYOffset;

    float left = x;
    float right = x + width;

    if (update){
 	    float percentage = (getGlobalState().xNdc - left) / (right - left);
 	    slider.percentage = percentage;
 	    update = false;
    }
    std::cout << "radio 1: " << slider.percentage << ", left = " << left << ", right = " << right << ", xndc = " << getGlobalState().xNdc << std::endl;

    drawRight(x, y, width, height, glm::vec4(0.f, 0.f, 0.f, .8f));
    drawRight(x, y, width * glm::min(1.f, glm::max(slider.percentage, 0.f)), height, glm::vec4(0.4f, 0.4f, 0.4f, .8f));

    BoundingBox2D boundingBox {
  		.x = x + (width * 0.5f),
  		.y = y,
  		.width = width,
  		.height = height,
		};

		drawDebugBoundingBox(boundingBox);
    return boundingBox;
  },
  .imMouseSelect = [](std::optional<objid> mappingIdSelected) -> void {
  	if (mappingIdSelected.has_value() && mappingIdSelected.value() == slider.mappingId){
	  	update = true;
  	}
  }  
};