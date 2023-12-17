#include "./style.h"

const bool DEBUG_STYLES = false;

glm::vec4 primaryColor;

Styles styles {
	.primaryColor = glm::vec4(0.f, 0.f, 0.f, 0.6f),
	.highlightColor = glm::vec4(0.f, 0.f, 1.f, 1.f),
	.debugColor = DEBUG_STYLES ? glm::vec4(1.f, 0.f, 0.f, 1.f) : glm::vec4(0.f, 0.f, 0.f, 0.f),
	.mainBorderColor = glm::vec4(0.f, 0.f, 0.f, 1.f),
};
