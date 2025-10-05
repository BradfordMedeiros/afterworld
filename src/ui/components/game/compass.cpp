#include "./compass.h"

extern CustomApiBindings* gameapi;

const float rectX = 0.f;
const float rectY = 0.f;
const float rectWidth = 0.5f;
const float rectHeight = 0.5f;
const float compassNeedleLength = 0.1f;

Component compassComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
  	auto mappingId = uniqueMenuItemMappingId();
    drawTools.drawRect(rectX, rectY, rectWidth, rectHeight, false, glm::vec4(0.f, 0.f, 1.f, 0.1f), true, mappingId, std::nullopt, std::nullopt, std::nullopt);
    BoundingBox2D boundingBox {
      .x = rectX,
      .y = rectY,
      .width  = rectWidth,
      .height = rectHeight,
    };

    auto cameraTransform = gameapi -> getCameraTransform(0).rotation; // TODO viewport
    auto orientationVec  = glm::inverse(cameraTransform) *  glm::vec4(0.f, 0.f, -1.f, 0.f);
    auto angleRadians = atanRadians360(orientationVec.x, orientationVec.z);
    auto xPos = glm::cos(angleRadians);
    auto yPos = glm::sin(angleRadians);

    drawTools.drawLine2D(glm::vec3(0.f, 0.f, 0.f), glm::normalize(glm::vec3(xPos, yPos, 0.f)) * compassNeedleLength, false, glm::vec4(1.f, 0.f, 0.f, 1.f), true, mappingId, std::nullopt, std::nullopt);

	 	drawDebugBoundingBox(drawTools, boundingBox, glm::vec4(1.f, 1.f, 1.f, 0.5f));
    return boundingBox;
  },
};
