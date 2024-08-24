#include "./weaponwheel.h"

const int RESOLUTION = 20;
const float rectX = 0.f;
const float rectY = 0.f;
const float rectWidth = 0.5f;
const float rectHeight = 0.5f;

float angleToDegrees(float x, float y){
	float atanValue = glm::atan(y / x);
	std::cout  << "atan: " << atanValue << std::endl;
	std::cout << "degrees: " << glm::degrees(atanValue) << std::endl;

	auto degrees = glm::degrees(atanValue);
	
	if (y > 0 && x < 0){
		degrees = 180 - (-1 * glm::degrees(atanValue));
	}else if (y < 0 && x > 0){
		degrees = 360 + glm::degrees(atanValue);
	}else if (y < 0 && x < 0) {
		degrees = 180 + (1 * glm::degrees(atanValue));
	}

	std::cout << "degrees = " << degrees << ", radians = " << glm::radians(degrees) << std::endl;
	return degrees;
}
int degreesToPartition(float degrees, int numPartitions){
	if (degrees < 0){
		return 0;
	}
	std::cout << "weaponwheel angle: " << degrees << std::endl;
	auto degreesPerPartition = 360.f / numPartitions;
	std::cout << "degrees per partition: " << degreesPerPartition << std::endl;
	auto partition = glm::round(degrees / degreesPerPartition);
	return partition;
}

int currentPartition(float x, float y){
	auto partition = degreesToPartition(angleToDegrees(x, y), RESOLUTION);
	std::cout << "partition: " << partition << std::endl;
	return partition;
}

void drawCirclePartition(DrawingTools& drawTools, int i, int mappingId, glm::vec4 tint){
  float value = i * (2 * M_PI) / RESOLUTION;
  float nextValue = (i + 1) * (2 * M_PI) / RESOLUTION;
  glm::vec3 fromPos(rectWidth * glm::cos(value), rectHeight * glm::sin(value), 0.f);
  glm::vec3 toPos(rectWidth * glm::cos(nextValue), rectHeight * glm::sin(nextValue), 0.f);
  drawTools.drawLine2D(fromPos, toPos, false, tint, true, mappingId, std::nullopt, std::nullopt);
  drawTools.drawLine2D(glm::vec3(0.f, 0.f, 0.f), fromPos, false, tint, true, mappingId, std::nullopt, std::nullopt);
  drawTools.drawLine2D(glm::vec3(0.f, 0.f, 0.f), toPos, false, tint, true, mappingId, std::nullopt, std::nullopt);
}

Component weaponWheelComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
  	auto mappingId = uniqueMenuItemMappingId();
  	auto partitionIndex = currentPartition(getGlobalState().xNdc, getGlobalState().yNdc);
  	for (int i = 0; i < RESOLUTION; i++){
  		auto selectedPartitionIndex = partitionIndex == i;
  		if (selectedPartitionIndex){
  			continue;
  		}
  		drawCirclePartition(drawTools, i, mappingId, glm::vec4(1.f, 1.f, 1.f, 1.f));
  	}
  	drawCirclePartition(drawTools, partitionIndex, mappingId, glm::vec4(0.f, 1.f, 0.f, 1.f));

  	return BoundingBox2D {
  		.x = rectX,
  		.y = rectY,
  		.width = rectWidth,
  		.height = rectHeight,
  	};
  },
};
