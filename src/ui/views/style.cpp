#include "./style.h"

const bool DEBUG_STYLES = false;

Styles styles {
	.primaryColor = glm::vec4(0.f, 0.f, 0.f, 0.5f),
	.secondaryColor = DEBUG_STYLES ? glm::vec4(0.f, 0.f, 1.f, 1.f) : glm::vec4(0.2f, 0.2f, 0.2f, 0.5f),
	.thirdColor = DEBUG_STYLES ? glm::vec4(0.f, 1.f, 1.f, 1.f) : glm::vec4(0.4f, 0.4f, 0.4f, 0.5f),
	.highlightColor = glm::vec4(0.f, 1.f, 0.f, 0.5f),
	.debugColor = DEBUG_STYLES ? glm::vec4(0.f, 0.f, 0.f, 1.f) : glm::vec4(0.f, 0.f, 0.f, 0.f),
	.debugColor2 = DEBUG_STYLES ? glm::vec4(0.f, 0.f, 0.f, 1.f) : glm::vec4(0.f, 0.f, 0.f, 0.f),
	.mainBorderColor = glm::vec4(0.f, 0.f, 0.f, 1.f),

	.dockElementPadding = 0.02f,
};
