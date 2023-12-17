#include "./style.h"

const bool DEBUG_STYLES = false;


Styles styles {
	.primaryColor = glm::vec4(0.f, 0.f, 0.f, 0.6f),
	.secondaryColor = DEBUG_STYLES ? glm::vec4(0.f, 0.f, 1.f, 1.f) : glm::vec4(0.2f, 0.2f, 0.2f, 1.f),
	.highlightColor = glm::vec4(0.f, 0.f, 1.f, 1.f),
	.debugColor = DEBUG_STYLES ? glm::vec4(1.f, 0.f, 0.f, 1.f) : glm::vec4(0.f, 0.f, 0.f, 0.f),
	.mainBorderColor = glm::vec4(0.f, 0.f, 0.f, 1.f),
};
